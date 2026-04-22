#ifndef TTYPRLAND_SERVER_H
#define TTYPRLAND_SERVER_H

#include <stdbool.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_scene.h>
#include <wayland-server-core.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_output_layout.h>

struct ttypr_server;

struct ttypr_server_graphics {
  struct ttypr_server *server;

  struct wlr_backend   *backend;
  struct wlr_renderer  *renderer;
  struct wlr_allocator *allocator;
};

struct ttypr_server_protocols {
  struct ttypr_server *server;

  struct wlr_compositor *compositor;
  struct wlr_subcompositor *subcompositor;
  struct wlr_data_device_manager *data_device_manager;
  struct wlr_xdg_shell *xdg_shell;

  struct wl_listener new_xdg_toplevel;
};

struct ttypr_server_input {
  struct ttypr_server *server;

  struct wlr_seat *seat;
};

struct ttypr_server_output {
  struct ttypr_server *server;

  struct wlr_output_layout *output_layout;
  struct wl_listener new_output;
};

struct ttypr_server_scene {
  struct ttypr_server *server;

  struct wlr_scene *scene;
  struct wlr_scene_output_layout *scene_output_layout;
};

struct ttypr_server {
  struct wl_display *display;
  const char *socket;

  struct ttypr_server_graphics  graphics;
  struct ttypr_server_protocols protocols;
  struct ttypr_server_input     input;
  struct ttypr_server_output    output;
  struct ttypr_server_scene     scene;
};

bool ttypr_server_graphics_init(struct ttypr_server_graphics *);
void ttypr_server_graphics_destroy(struct ttypr_server_graphics *);

bool ttypr_server_protocols_init(struct ttypr_server_protocols *);
void ttypr_server_protocols_destroy(struct ttypr_server_protocols *);

bool ttypr_server_input_init(struct ttypr_server_input *);
void ttypr_server_input_destroy(struct ttypr_server_input *);

bool ttypr_server_output_init(struct ttypr_server_output *);
void ttypr_server_output_destroy(struct ttypr_server_output *);

bool ttypr_server_scene_init(struct ttypr_server_scene *);
void ttypr_server_scene_destroy(struct ttypr_server_scene *);

bool ttypr_server_init(struct ttypr_server *);
void ttypr_server_destroy(struct ttypr_server *);

#endif // TTYPRLAND_SERVER_H
