#include "signals.h"
#include "common.h"

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
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);
}
