#ifndef TTYPRLAND_OUTPUT_H
#define TTYPRLAND_OUTPUT_H

#include <wayland-server-core.h>
#include <ttyprland/server.h>

struct ttypr_output {
  struct ttypr_server *server;

  struct wlr_output *wlr_output;

  struct wl_listener frame;
  struct wl_listener request_state;
  struct wl_listener destroy;
};

#endif // TTYPRLAND_OUTPUT_H
