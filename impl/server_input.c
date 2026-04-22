#include <ttyprland/server.h>
#include <wlr/util/log.h>

bool ttypr_server_input_init(struct ttypr_server_input *server_input) {
  struct ttypr_server *server = server_input->server;

  wlr_log(WLR_DEBUG, "ttypr_server_input_init() started");

  server_input->seat = wlr_seat_create(server->display, "i-have-the-seat-full-of-water");
  if (!server_input->seat) {
    wlr_log(WLR_ERROR, "wlr_seat_create() failed");
    goto error_seat;
  }

  wlr_log(WLR_DEBUG, "wlr_seat_create() succeeded");

  wlr_log(WLR_INFO, "ttypr_server_input_init() finished");

  return true;

error_seat:
  return false;
}

void ttypr_server_input_destroy(struct ttypr_server_input *server_input) {
  if (server_input->seat) wlr_seat_destroy(server_input->seat);

  server_input->seat = NULL;

  wlr_log(WLR_INFO, "ttypr_server_input destroyed");
}
