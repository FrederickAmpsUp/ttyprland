#ifndef TTYPRLAND_TOPLEVEL_H
#define TTYPRLAND_TOPLEVEL_H

#include <ttyprland/server.h>

struct ttypr_toplevel {
  struct ttypr_server *server;

  struct wlr_xdg_toplevel *xdg_toplevel;
  struct wlr_scene_tree *scene_tree;

  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener commit;
  struct wl_listener destroy;
};

#endif // TTYPRLAND_TOPLEVEL_H
