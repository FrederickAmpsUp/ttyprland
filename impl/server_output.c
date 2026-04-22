#include <ttyprland/server.h>
#include <wlr/util/log.h>

bool ttypr_server_output_init(struct ttypr_server_output *server_output) {
  struct ttypr_server *server = server_output->server;

  wlr_log(WLR_DEBUG, "ttypr_server_output_init() started");

  server_output->output_layout = wlr_output_layout_create(server->display);
  if (!server_output->output_layout) {
    wlr_log(WLR_ERROR, "wlr_output_layout_create() failed");
    goto error_layout;
  }

  wlr_log(WLR_DEBUG, "wlr_output_layout_create() succeeded");

  wlr_log(WLR_INFO, "ttypr_server_output_init() finished");

  return true;

error_layout:
  return false;
}

void ttypr_server_output_destroy(struct ttypr_server_output *server_output) {
  if (server_output->output_layout) wlr_output_layout_destroy(server_output->output_layout);

  server_output->output_layout = NULL;

  wlr_log(WLR_INFO, "ttypr_server_output destroyed");
}
