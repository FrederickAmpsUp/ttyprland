#include <ttyprland/server.h>
#include <ttyprland/output.h>
#include <wlr/util/log.h>
#include <stdlib.h>
#include <time.h>

static void on_frame(struct wl_listener *listener, void *data) {
  struct ttypr_output *output = wl_container_of(listener, output, frame);

  if (!output->server->scene.scene) {
    wlr_log(WLR_ERROR, "no scene to draw");
    return;
  }

  if (!output->output) {
    wlr_log(WLR_ERROR, "no valid output");
    return;
  }

  struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(
    output->server->scene.scene, output->output);

  if (!scene_output) {
    wlr_log(WLR_ERROR, "wlr_scene_get_scene_output() failed");
    return;
  }
  
  wlr_scene_output_commit(scene_output, NULL);

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  wlr_scene_output_send_frame_done(scene_output, &now);
}

static void on_request_state(struct wl_listener *listener, void *data) {
  struct ttypr_output *output = wl_container_of(listener, output, request_state);
  const struct wlr_output_event_request_state *event = data;
  wlr_output_commit_state(output->output, event->state);
}

static void on_destroy(struct wl_listener *listener, void *data) {
  struct ttypr_output *output = wl_container_of(listener, output, destroy);

  wlr_log(WLR_DEBUG, "output '%s' destroyed.", output->output->name);

  wl_list_remove(&output->frame.link);
  wl_list_remove(&output->request_state.link);
  wl_list_remove(&output->destroy.link);
  free(output);
}

static void on_new_output(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "on_new_output() started");

  struct ttypr_server_output *server_output = wl_container_of(listener, server_output, new_output);
  struct ttypr_server *server = server_output->server;
  struct wlr_output *output = data;

  if(!wlr_output_init_render(output, server->graphics.allocator, server->graphics.renderer)) {
    wlr_log(WLR_ERROR, "wlr_output_init_render() failed");
    abort();
  }

  wlr_log(WLR_DEBUG, "wlr_output_init_render() succeeded");

  struct wlr_output_state output_state;
  wlr_output_state_init(&output_state);
  wlr_output_state_set_enabled(&output_state, true);

  struct wlr_output_mode *output_mode = wlr_output_preferred_mode(output);
  if (output_mode) {
    wlr_output_state_set_mode(&output_state, output_mode);
  }

  wlr_output_commit_state(output, &output_state);
  wlr_output_state_finish(&output_state);

  wlr_log(WLR_INFO, "new output '%s' using %dx%d@%.2fHz",
    output->name? output->name : "unknown",
    output_mode ? output_mode->width : 0,
    output_mode ? output_mode->height : 0,
    output_mode ? output_mode->refresh / 1000.0 : 0.0);

  struct ttypr_output *ttypr_output = calloc(1, sizeof(*ttypr_output));
  if (!ttypr_output) {
    wlr_log(WLR_ERROR, "calloc() failed");
    abort();
  }

  ttypr_output->server = server;
  ttypr_output->output = output;

  ttypr_output->frame.notify = on_frame;
  wl_signal_add(&output->events.frame, &ttypr_output->frame);

  ttypr_output->request_state.notify = on_request_state;
  wl_signal_add(&output->events.request_state, &ttypr_output->request_state);

  ttypr_output->destroy.notify = on_destroy;
  wl_signal_add(&output->events.destroy, &ttypr_output->destroy);

  wlr_log(WLR_DEBUG, "event listeners bound");

  struct wlr_output_layout_output *output_layout_output = wlr_output_layout_add_auto(server_output->output_layout, output);
  if (!output_layout_output) {
    wlr_log(WLR_ERROR, "wlr_output_layout_add_auto() failed");
    abort();
  }

  wlr_log(WLR_DEBUG, "wlr_output_layout_add_auto() succeeded");

  struct wlr_scene_output *scene_output = wlr_scene_output_create(server->scene.scene, output);
  if (!scene_output) {
    wlr_log(WLR_ERROR, "wlr_scene_output_create() failed");
    abort();
  }

  wlr_log(WLR_DEBUG, "wlr_scene_output_create() succeeded");

  wlr_scene_output_layout_add_output(server->scene.scene_output_layout, output_layout_output, scene_output);
  wlr_scene_output_set_position(scene_output, 0, 0);

  wlr_log(WLR_INFO, "on_new_output() finished");
}

bool ttypr_server_output_init(struct ttypr_server_output *server_output) {
  struct ttypr_server *server = server_output->server;

  wlr_log(WLR_DEBUG, "ttypr_server_output_init() started");

  server_output->output_layout = wlr_output_layout_create(server->display);
  if (!server_output->output_layout) {
    wlr_log(WLR_ERROR, "wlr_output_layout_create() failed");
    goto error_layout;
  }

  wlr_log(WLR_DEBUG, "wlr_output_layout_create() succeeded");

  server_output->new_output.notify = on_new_output;
  wl_signal_add(&server->graphics.backend->events.new_output, &server_output->new_output);

  wlr_log(WLR_DEBUG, "event listeners bound");

  wlr_log(WLR_INFO, "ttypr_server_output_init() finished");

  return true;

error_layout:
  return false;
}

void ttypr_server_output_destroy(struct ttypr_server_output *server_output) {
  wl_list_remove(&server_output->new_output.link);
  wl_list_init(&server_output->new_output.link);

  if (server_output->output_layout) wlr_output_layout_destroy(server_output->output_layout);

  server_output->output_layout = NULL;

  wlr_log(WLR_INFO, "ttypr_server_output destroyed");
}
