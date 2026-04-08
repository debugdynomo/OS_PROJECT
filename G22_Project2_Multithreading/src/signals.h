/*
 * signals.h — Signal handler setup
 * Owner: Member 1 (VISHNU TEJA VASAM)
 *
 * Installs handlers for SIGINT (graceful shutdown) and
 * SIGUSR1 (print active thread count).
 * -------------------------------------------------------------------
 * DO NOT MODIFY this file without coordinating with Member 1.
 */

#ifndef SIGNALS_H
#define SIGNALS_H

/*
 * Install signal handlers. Call once in main() before the event loop.
 * - SIGINT  → sets g_shutdown = 1  (graceful exit)
 * - SIGUSR1 → prints active thread count to stderr
 */
void setup_signal_handlers(void);

#endif /* SIGNALS_H */
