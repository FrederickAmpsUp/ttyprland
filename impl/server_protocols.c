#include <ttyprland/server.h>
#include <wlr/util/log.h>

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

  wlr_log(WLR_INFO, "ttypr_server_protocols_init() finished");

  return true;

error_xdg_shell:
error_data_device_manager:
error_subcompositor:
error_compositor:
  return false;
}

void ttypr_server_protocols_destroy(struct ttypr_server_protocols *server_protocols) {
  server_protocols->xdg_shell = NULL;
  server_protocols->data_device_manager = NULL;
  server_protocols->subcompositor = NULL;
  server_protocols->compositor = NULL;

  wlr_log(WLR_INFO, "ttypr_server_protocols destroyed");
}
