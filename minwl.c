#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include <wayland-server-core.h>

#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>

struct server {
  struct wl_display *display;
  const char *socket;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;
  struct wlr_allocator *allocator;

  struct wlr_compositor *compositor;
  struct wlr_subcompositor *subcompositor;
  struct wlr_data_device_manager *data_device_manager;
  struct wlr_output_layout *output_layout;

  struct wlr_seat *seat;

  struct wlr_xdg_shell *xdg_shell;

  struct wlr_scene *scene;
  struct wlr_scene_output_layout *scene_layout;

  struct wl_listener new_xdg_toplevel;
  struct wl_listener new_output;
};

struct output {
  struct server *server;
  struct wlr_output *wlr_output;

  struct wl_listener frame;
  struct wl_listener request_state;
  struct wl_listener destroy;
};

struct toplevel {
  struct server *server;

  struct wlr_xdg_toplevel *xdg_toplevel;
  struct wlr_scene_tree *scene_tree;

  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener commit;
  struct wl_listener destroy;
};

static void on_frame(struct wl_listener *listener, void *data) {
  struct output *out = wl_container_of(listener, out, frame);
  struct wlr_output *wo = out->wlr_output;

  struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(out->server->scene, wo);

  wlr_scene_output_commit(scene_output, NULL);

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  wlr_scene_output_send_frame_done(scene_output, &now);
  wlr_output_schedule_frame(wo);
}

static void on_request_state(struct wl_listener *listener, void *data) {
  struct output *out = wl_container_of(listener, out, request_state);
  const struct wlr_output_event_request_state *event = data;
  wlr_output_commit_state(out->wlr_output, event->state);
}

static void on_output_destroy(struct wl_listener *listener, void *data) {
  struct output *out = wl_container_of(listener, out, destroy);
  wl_list_remove(&out->frame.link);
  wl_list_remove(&out->request_state.link);
  wl_list_remove(&out->destroy.link);
  free(out);
}

static void on_new_output(struct wl_listener *listener, void *data) {
  struct server *srv = wl_container_of(listener, srv, new_output);
  struct wlr_output *wo = data;

  wlr_output_init_render(wo, srv->allocator, srv->renderer);

  struct wlr_output_state state;
  wlr_output_state_init(&state);
  wlr_output_state_set_enabled(&state, true);

  struct wlr_output_mode *mode = wlr_output_preferred_mode(wo);
  if (mode) {
    wlr_output_state_set_mode(&state, mode);
  }

  wlr_output_commit_state(wo, &state);
  wlr_output_state_finish(&state);

  struct output *out = calloc(1, sizeof(*out));
  out->server = srv;
  out->wlr_output = wo;

  out->frame.notify = on_frame;
  wl_signal_add(&wo->events.frame, &out->frame);

  out->request_state.notify = on_request_state;
  wl_signal_add(&wo->events.request_state, &out->request_state);

  out->destroy.notify = on_output_destroy;
  wl_signal_add(&wo->events.destroy, &out->destroy);

  struct wlr_output_layout_output *l_output = wlr_output_layout_add_auto(srv->output_layout, wo);
  struct wlr_scene_output *scene_output = wlr_scene_output_create(srv->scene, wo);
  wlr_scene_output_layout_add_output(srv->scene_layout, l_output, scene_output);
  wlr_scene_output_set_position(scene_output, 0, 0);

  wlr_log(WLR_INFO, "New output: %s (%dx%d)", wo->name, wo->width, wo->height);
}

static void on_xdg_surface_map(struct wl_listener *listener, void *data) {
  struct toplevel *toplevel = wl_container_of(listener, toplevel, map);

  toplevel->scene_tree = wlr_scene_xdg_surface_create(
    &toplevel->server->scene->tree,
    toplevel->xdg_toplevel->base
  );
  
  wlr_scene_node_set_position(&toplevel->scene_tree->node, 0, 0);

  wlr_log(WLR_INFO, "toplevel mapped");
}

static void on_xdg_surface_unmap(struct wl_listener *listener, void *data) {
  struct toplevel *toplevel = wl_container_of(listener, toplevel, unmap);

  if (toplevel->scene_tree) {
    wlr_scene_node_destroy(&toplevel->scene_tree->node);
    toplevel->scene_tree = NULL;
  }
}

static void on_xdg_surface_commit(struct wl_listener *listener, void *data) {
  struct toplevel *toplevel = wl_container_of(listener, toplevel, commit);

  wlr_log(WLR_INFO, "surface commit");

  if (toplevel->xdg_toplevel->base->initial_commit) {
    wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
    wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
  }
}

static void on_xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
  struct toplevel *toplevel = wl_container_of(listener, toplevel, destroy);
  
  wl_list_remove(&toplevel->map.link);
  wl_list_remove(&toplevel->unmap.link);
  wl_list_remove(&toplevel->commit.link);
  wl_list_remove(&toplevel->destroy.link);

  if (toplevel->scene_tree) {
    wlr_scene_node_destroy(&toplevel->scene_tree->node);
    toplevel->scene_tree = NULL;
  }
  
  free(toplevel);
}

static void on_new_xdg_toplevel(struct wl_listener *listener, void *data) {
  struct server *srv = wl_container_of(listener, srv, new_xdg_toplevel);
  struct wlr_xdg_toplevel *xdg_toplevel = data;

  struct toplevel *toplevel = calloc(1, sizeof(*toplevel));
  toplevel->server = srv;
  toplevel->xdg_toplevel = xdg_toplevel;

  toplevel->map.notify = on_xdg_surface_map;
  wl_signal_add(&xdg_toplevel->base->surface->events.map, &toplevel->map);

  toplevel->unmap.notify = on_xdg_surface_unmap;
  wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &toplevel->unmap);

  toplevel->commit.notify = on_xdg_surface_commit;
  wl_signal_add(&xdg_toplevel->base->surface->events.commit, &toplevel->commit);

  toplevel->destroy.notify = on_xdg_toplevel_destroy;
  wl_signal_add(&xdg_toplevel->events.destroy, &toplevel->destroy);

  wlr_log(WLR_INFO, "new toplevel created");
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(void) {
  wlr_log_init(WLR_DEBUG, NULL);

  struct server srv = {0};

  srv.display = wl_display_create();

  srv.socket = wl_display_add_socket_auto(srv.display);
  if (!srv.socket) {
    wlr_log(WLR_ERROR, "wlr_display_add_socket_auto failed");
    return 1;
  }

  srv.backend =
      wlr_backend_autocreate(wl_display_get_event_loop(srv.display), NULL);
  if (!srv.backend) {
    wlr_log(WLR_ERROR, "wlr_backend_autocreate failed");
    return 1;
  }

  srv.renderer = wlr_renderer_autocreate(srv.backend);
  if (!srv.renderer) {
    wlr_log(WLR_ERROR, "wlr_renderer_autocreate failed");
    return 1;
  }
  wlr_renderer_init_wl_display(srv.renderer, srv.display);

  srv.allocator = wlr_allocator_autocreate(srv.backend, srv.renderer);
  if (!srv.allocator) {
    wlr_log(WLR_ERROR, "wlr_allocator_autocreate failed");
    return 1;
  }

  srv.new_output.notify = on_new_output;
  wl_signal_add(&srv.backend->events.new_output, &srv.new_output);

  // TODO: error checking
  srv.compositor = wlr_compositor_create(srv.display, 5, srv.renderer);
  srv.subcompositor = wlr_subcompositor_create(srv.display);
  srv.data_device_manager = wlr_data_device_manager_create(srv.display);

  srv.output_layout = wlr_output_layout_create(srv.display);

  srv.seat = wlr_seat_create(srv.display, "seat0");
  wlr_seat_set_capabilities(srv.seat, WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD);

  srv.xdg_shell = wlr_xdg_shell_create(srv.display, 3);

  srv.scene = wlr_scene_create();
  srv.scene_layout = wlr_scene_attach_output_layout(srv.scene, srv.output_layout);

  srv.new_xdg_toplevel.notify = on_new_xdg_toplevel;
  wl_signal_add(&srv.xdg_shell->events.new_toplevel, &srv.new_xdg_toplevel);

  if (!wlr_backend_start(srv.backend)) {
    wlr_log(WLR_ERROR, "wlr_backend_start failed");
    return 1;
  }

  wlr_log(WLR_INFO, "Running on %s - Ctrl-C to quit", srv.socket);
  wl_display_run(srv.display);

  wlr_allocator_destroy(srv.allocator);
  wlr_renderer_destroy(srv.renderer);
  wlr_backend_destroy(srv.backend);
  wl_display_destroy(srv.display);
  return 0;
}
