#include <ttyprland/server.h>
#include <wlr/util/log.h>

bool ttypr_server_graphics_init(struct ttypr_server_graphics *server_graphics) {
  wlr_log(WLR_DEBUG, "ttypr_server_graphics_init() started");

  server_graphics->backend = wlr_backend_autocreate(
    wl_display_get_event_loop(server_graphics->server->display),
    NULL
  );
  if (!server_graphics->backend) {
    wlr_log(WLR_ERROR, "wlr_backend_autocreate() failed");
    goto error_backend;
  }

  wlr_log(WLR_DEBUG, "wlr_backend_autocreate() succeeded");

  server_graphics->renderer = wlr_renderer_autocreate(server_graphics->backend);
  if (!server_graphics->renderer) {
    wlr_log(WLR_ERROR, "wlr_renderer_autocreate() failed");
    goto error_renderer;
  }

  wlr_log(WLR_DEBUG, "wlr_renderer_autocreate() succeeded");

  if (!wlr_renderer_init_wl_display(server_graphics->renderer, server_graphics->server->display)) {
    wlr_log(WLR_ERROR, "wlr_renderer_init_wl_display() failed");
    goto error_renderer_display;
  }

  wlr_log(WLR_DEBUG, "wlr_renderer_init_wl_display() succeded");

  server_graphics->allocator = wlr_allocator_autocreate(
    server_graphics->backend, server_graphics->renderer);
  if (!server_graphics->allocator) {
    wlr_log(WLR_ERROR, "wlr_allocator_autocreate() failed");
    goto error_allocator;
  }

  wlr_log(WLR_DEBUG, "wlr_allocator_autocreate() succeeded");

  wlr_log(WLR_INFO, "ttypr_server_graphics_init() finished");

  return true;

error_allocator:
error_renderer_display:
  wlr_renderer_destroy(server_graphics->renderer);
  server_graphics->renderer = NULL;
error_renderer:
  wlr_backend_destroy(server_graphics->backend);
  server_graphics->backend = NULL;
error_backend:
  return false;
}

void ttypr_server_graphics_destroy(struct ttypr_server_graphics *server_graphics) {
  if (server_graphics->allocator) wlr_allocator_destroy(server_graphics->allocator);
  if (server_graphics->renderer)  wlr_renderer_destroy(server_graphics->renderer);
  if (server_graphics->backend)   wlr_backend_destroy(server_graphics->backend);

  server_graphics->allocator = NULL;
  server_graphics->renderer = NULL;
  server_graphics->backend = NULL;

  wlr_log(WLR_INFO, "ttypr_server_graphics destroyed");
}
