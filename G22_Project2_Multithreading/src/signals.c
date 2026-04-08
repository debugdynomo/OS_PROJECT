/*
 * signals.c — Signal handler implementation
 * Owner: Member 1 (VISHNU TEJA VASAM)
 *
 * -------------------------------------------------------------------
 * DO NOT MODIFY this file without coordinating with Member 1.
 */

#include "signals.h"
#include "common.h"

/* ── Global shutdown flag ──────────────────────────────────────── */

volatile sig_atomic_t g_shutdown = 0;

/* ── Handlers ──────────────────────────────────────────────────── */

static void sigint_handler(int sig)
{
    (void)sig;
    g_shutdown = 1;
    /* write() is async-signal-safe, printf is NOT */
    const char msg[] = "\n[signal] SIGINT received — shutting down gracefully...\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

/* ── Setup ─────────────────────────────────────────────────────── */

void setup_signal_handlers(void)
{
    struct sigaction sa;

    /* SIGINT → graceful shutdown */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;    /* no SA_RESTART — let blocking calls return */
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("[signal] Failed to install SIGINT handler");
    }

    /* Ignore SIGPIPE so writing to closed pipes doesn't kill us */
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);
}
