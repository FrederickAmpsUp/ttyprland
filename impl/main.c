#include <ttyprland/server.h>
#include <wlr/util/log.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static struct ttypr_server *g_server = NULL;

static volatile sig_atomic_t dying = 0;
static void handle_signal(int sig) {
  (void)sig;

  if (dying) {
    _exit(1); // hard escape if already shutting down
  }

  dying = 1;

  if (g_server && g_server->display) {
    wl_display_terminate(g_server->display);
  } else {
    _exit(1);
  }
}
static void install_signal_handlers(void) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));

  sa.sa_handler = handle_signal;

  sigemptyset(&sa.sa_mask);

  sa.sa_flags = SA_RESTART;

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
}

int main(int argc, char **argv) {
  wlr_log_init(WLR_DEBUG, NULL);

  install_signal_handlers();

  struct ttypr_server server = { 0 };
  g_server = &server;
  ttypr_server_init(&server);

  wlr_backend_start(server.graphics.backend);

  wl_display_run(server.display);

  ttypr_server_destroy(&server);
  g_server = NULL;

  return 0;
}
