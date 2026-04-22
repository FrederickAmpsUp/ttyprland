#include <ttyprland/server.h>
#include <wlr/util/log.h>

int main(int argc, char **argv) {
  wlr_log_init(WLR_DEBUG, NULL);

  struct ttypr_server server = { 0 };
  ttypr_server_init(&server);

  ttypr_server_destroy(&server);

  return 0;
}
