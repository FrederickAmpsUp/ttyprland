#include <ttyprland/server.h>
#include <wlr/util/log.h>

bool ttypr_server_scene_init(struct ttypr_server_scene *server_scene) {
  struct ttypr_server *server = server_scene->server;

  wlr_log(WLR_DEBUG, "ttypr_server_scene_init() started");

  server_scene->scene = wlr_scene_create();
  if (!server_scene->scene) {
    wlr_log(WLR_ERROR, "wlr_scene_create() failed");
    goto error_scene;
  }

  wlr_log(WLR_DEBUG, "wlr_scene_create() succeeded");

  server_scene->scene_output_layout = wlr_scene_attach_output_layout(
    server_scene->scene, server->output.output_layout);
  if (!server_scene->scene_output_layout) {
    wlr_log(WLR_ERROR, "wlr_scene_attach_output_layout() failed");
    goto error_scene_output_layout;
  }

  wlr_log(WLR_INFO, "ttypr_server_scene_init() finished");

  return true;

error_scene_output_layout:
  wlr_scene_node_destroy(&server_scene->scene->tree.node);
  server_scene->scene = NULL;
error_scene:
  return false;
}

void ttypr_server_scene_destroy(struct ttypr_server_scene *server_scene) {
  if (server_scene->scene) wlr_scene_node_destroy(&server_scene->scene->tree.node);

  server_scene->scene = NULL;

  wlr_log(WLR_INFO, "ttypr_server_scene destroyed");
}
