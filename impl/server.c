#include <ttyprland/server.h>
#include <wlr/util/log.h>

bool ttypr_server_init(struct ttypr_server *server) {
  wlr_log(WLR_DEBUG, "ttypr_server_init() started");

  server->display = wl_display_create();
  if (!server->display) {
    wlr_log(WLR_ERROR, "wl_display_create() failed");
    goto error_display;
  }

  wlr_log(WLR_DEBUG, "wl_display_create() succeeded");

  server->socket = wl_display_add_socket_auto(server->display);
  if (!server->socket) {
    wlr_log(WLR_ERROR, "wl_display_add_socket_auto() failed");
    goto error_socket;
  }

  wlr_log(WLR_DEBUG, "wl_display_add_socket_auto() succeeded");

  server->graphics.server = server;
  if (!ttypr_server_graphics_init(&server->graphics)) {
    wlr_log(WLR_ERROR, "ttypr_server_graphics_init() failed");
    goto error_graphics;
  }

  wlr_log(WLR_DEBUG, "ttypr_server_graphics_init() succeeded");

  server->protocols.server = server;
  if (!ttypr_server_protocols_init(&server->protocols)) {
    wlr_log(WLR_ERROR, "ttypr_server_protocols_init() failed");
    goto error_protocols;
  }

  wlr_log(WLR_DEBUG, "ttypr_server_protocols_init() succeeded");

  server->input.server = server;
  if (!ttypr_server_input_init(&server->input)) {
    wlr_log(WLR_ERROR, "ttypr_server_input_init() failed");
    goto error_input;
  }

  wlr_log(WLR_DEBUG, "ttypr_server_input_init() succeeded");

  server->output.server = server;
  if (!ttypr_server_output_init(&server->output)) {
    wlr_log(WLR_ERROR, "ttypr_server_output_init() failed");
    goto error_output;
  }

  wlr_log(WLR_DEBUG, "ttypr_server_output_init() succeeded");

  server->scene.server = server;
  if (!ttypr_server_scene_init(&server->scene)) {
    wlr_log(WLR_ERROR, "ttypr_server_scene_init() failed");
    goto error_scene;
  }

  wlr_log(WLR_DEBUG, "ttypr_server_scene_init() succeeded");

  wlr_log(WLR_INFO, "ttypr_server_init() finished");

  return true;

error_scene:
  ttypr_server_output_destroy(&server->output);
error_output:
  ttypr_server_input_destroy(&server->input);
error_input:
  ttypr_server_protocols_destroy(&server->protocols);
error_protocols:
  ttypr_server_graphics_destroy(&server->graphics);
error_graphics:
error_socket:
  wl_display_destroy(server->display);
error_display:
  return false;
}

void ttypr_server_destroy(struct ttypr_server *server) {
  ttypr_server_scene_destroy(&server->scene);
  ttypr_server_output_destroy(&server->output);
  ttypr_server_input_destroy(&server->input);
  ttypr_server_protocols_destroy(&server->protocols);
  ttypr_server_graphics_destroy(&server->graphics);
  if (server->display) wl_display_destroy(server->display);

  server->display = NULL;

  wlr_log(WLR_INFO, "ttypr_server destroyed");
}
