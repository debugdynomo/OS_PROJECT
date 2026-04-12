#include "signals.h"
#include "common.h"
#include <signal.h>

volatile sig_atomic_t g_shutdown = 0;

static void sigint_handler(int sig)
{
    (void)sig;
    g_shutdown = 1;
    const char msg[] = "\nShutting down gracefully...\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

void setup_signal_handlers(void)
{
    signal(SIGINT, sigint_handler);
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif
}
