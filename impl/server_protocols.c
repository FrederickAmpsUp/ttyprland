#include <ttyprland/toplevel.h>
#include <ttyprland/server.h>
#include <wlr/util/log.h>
#include <stdlib.h>

static const char *toplevel_app_id(struct ttypr_toplevel *toplevel) {
  return toplevel->xdg_toplevel->app_id? toplevel->xdg_toplevel->app_id : "(no app_id)";
}

void on_toplevel_map(struct wl_listener *listener, void *data) {
  struct ttypr_toplevel *toplevel = wl_container_of(listener, toplevel, map);

  wlr_log(WLR_DEBUG, "on_toplevel_map() started");

  toplevel->scene_tree = wlr_scene_xdg_surface_create(
    &toplevel->server->scene.scene->tree,
    toplevel->xdg_toplevel->base
  );
  if (!toplevel->scene_tree) {
    wlr_log(WLR_ERROR, "wlr_scene_xdg_surface_create() failed for '%s'",
      toplevel_app_id(toplevel));
    return;
  }

  wlr_log(WLR_DEBUG, "wlr_scene_xdg_surface_create() succeeded");

  wlr_log(WLR_INFO, "toplevel '%s' mapped",
    toplevel_app_id(toplevel));
  wlr_log(WLR_INFO, "on_toplevel_map() finished");
}

void on_toplevel_unmap(struct wl_listener *listener, void *data) {
  struct ttypr_toplevel *toplevel = wl_container_of(listener, toplevel, unmap);

  wlr_log(WLR_DEBUG, "on_toplevel_unmap() started");

  if (toplevel->scene_tree) {
    wlr_scene_node_destroy(&toplevel->scene_tree->node);
    toplevel->scene_tree = NULL;

    wlr_log(WLR_DEBUG, "destroyed scene node for toplevel '%s'",
      toplevel_app_id(toplevel)); 
  } else {
    wlr_log(WLR_DEBUG, "nothing to do for toplevel '%s'",
      toplevel_app_id(toplevel));
  }

  wlr_log(WLR_INFO, "toplevel '%s' unmapped",
    toplevel_app_id(toplevel));
  wlr_log(WLR_INFO, "on_toplevel_unmap() finished");
}

void on_toplevel_commit(struct wl_listener *listener, void *data) {
  struct ttypr_toplevel *toplevel = wl_container_of(listener, toplevel, commit);

  wlr_log(WLR_DEBUG, "on_toplevel_commit() started");

  if (toplevel->xdg_toplevel->base->initial_commit) {
    wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
    wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
  
    wlr_log(WLR_INFO, "surface configured for toplevel '%s'",
      toplevel_app_id(toplevel));
  }

  wlr_log(WLR_INFO, "on_toplevel_commit() finished");
}

void on_toplevel_destroy(struct wl_listener *listener, void *data) {
  struct ttypr_toplevel *toplevel = wl_container_of(listener, toplevel, destroy);

  wlr_log(WLR_DEBUG, "on_toplevel_destroy() started");
  wlr_log(WLR_INFO, "destroying toplevel '%s'",
    toplevel_app_id(toplevel));

  wl_list_remove(&toplevel->map.link);
  wl_list_remove(&toplevel->unmap.link);
  wl_list_remove(&toplevel->commit.link);
  wl_list_remove(&toplevel->destroy.link);

  free(toplevel);

  wlr_log(WLR_INFO, "on_toplevel_destroy() finished");
}

void on_new_xdg_toplevel(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "on_new_xdg_toplevel() started");

  struct ttypr_server_protocols *server_protocols = wl_container_of(listener, server_protocols, new_xdg_toplevel);
  struct ttypr_server *server = server_protocols->server;

  struct wlr_xdg_toplevel *xdg_toplevel = data;

  struct ttypr_toplevel *toplevel = calloc(1, sizeof(*toplevel));
  if (!toplevel) {
    wlr_log(WLR_ERROR, "calloc() failed");
    abort();
  }

  toplevel->server = server;
  toplevel->xdg_toplevel = xdg_toplevel;

  wlr_log(WLR_INFO, "new toplevel '%s'",
    toplevel_app_id(toplevel));

  toplevel->scene_tree = NULL;

  toplevel->map.notify = on_toplevel_map;
  wl_signal_add(&xdg_toplevel->base->surface->events.map, &toplevel->map);

  toplevel->unmap.notify = on_toplevel_unmap;
  wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &toplevel->unmap);

  toplevel->commit.notify = on_toplevel_commit;
  wl_signal_add(&xdg_toplevel->base->surface->events.commit, &toplevel->commit);

  toplevel->destroy.notify = on_toplevel_destroy;
  wl_signal_add(&xdg_toplevel->base->surface->events.destroy, &toplevel->destroy);

  wlr_log(WLR_DEBUG, "event listeners bound");

  wlr_log(WLR_INFO, "on_new_xdg_toplevel() finished");
}

bool ttypr_server_protocols_init(struct ttypr_server_protocols *server_protocols) {
  struct ttypr_server *server = server_protocols->server;

  wlr_log(WLR_DEBUG, "ttypr_server_protocols_init() started");

  server_protocols->compositor = wlr_compositor_create(server->display, 5, server->graphics.renderer);
  if (!server_protocols->compositor) {
    wlr_log(WLR_ERROR, "wlr_compositor_create() failed");
    goto error_compositor;
  }

  wlr_log(WLR_DEBUG, "wlr_compositor_create() succeeded");

  server_protocols->subcompositor = wlr_subcompositor_create(server->display);
  if (!server_protocols->subcompositor) {
    wlr_log(WLR_ERROR, "wlr_subcompositor_create() failed");
    goto error_subcompositor;
  }

  wlr_log(WLR_DEBUG, "wlr_subcompositor_create() succeeded");

  server_protocols->data_device_manager = wlr_data_device_manager_create(server->display);
  if (!server_protocols->data_device_manager) {
    wlr_log(WLR_ERROR, "wlr_data_device_manager_create() failed");
    goto error_data_device_manager;
  }

  wlr_log(WLR_DEBUG, "wlr_data_device_manager_create() succeeded");

  server_protocols->xdg_shell = wlr_xdg_shell_create(server->display, 3);
  if (!server_protocols->xdg_shell) {
    wlr_log(WLR_ERROR, "wlr_xdg_shell_create() failed");
    goto error_xdg_shell;
  }

  wlr_log(WLR_DEBUG, "wlr_xdg_shell_create() succeeded");

  server_protocols->new_xdg_toplevel.notify = on_new_xdg_toplevel;
  wl_signal_add(&server_protocols->xdg_shell->events.new_toplevel, &server_protocols->new_xdg_toplevel);

  wlr_log(WLR_DEBUG, "event listeners bound");

  wlr_log(WLR_INFO, "ttypr_server_protocols_init() finished");

  return true;

error_xdg_shell:
error_data_device_manager:
error_subcompositor:
error_compositor:
  return false;
}

void ttypr_server_protocols_destroy(struct ttypr_server_protocols *server_protocols) {
  wl_list_remove(&server_protocols->new_xdg_toplevel.link);

  server_protocols->xdg_shell = NULL;
  server_protocols->data_device_manager = NULL;
  server_protocols->subcompositor = NULL;
  server_protocols->compositor = NULL;

  wlr_log(WLR_INFO, "ttypr_server_protocols destroyed");
}
