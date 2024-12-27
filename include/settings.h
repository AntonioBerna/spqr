#pragma once

#define AUTHORS "Antonio Bernardini & Flavio Caporilli"

#define SWAP_DIGITS(k) \
	(((k % 10) * 1000) + (((k / 10) % 10) * 100) + (((k / 100) % 10) * 10) + (k / 1000))

// * Configuration Parameters * //
#define PORT                6969
#define MAX_PORT            SWAP_DIGITS(PORT) // ? 9696
#define MAX_CLIENTS         (MAX_PORT - PORT) // ? 2727

#define CLIENT_PROMPT       "[client@spqr ~]$ "
#define SERVER_FILES_PATH   "server-files/"
#define CLIENT_FILES_PATH   "client-files/"
#define ASCII_ART_PATH      "assets/art/ascii.art"
#define PACKAGE_EMOJI       "📦"

#define TIMEOUT             8000  // ? Configurable Static Timeout (8 ms)
#define LOSS_PROBABILITY    25    // ? Change the probability of packet loss in range [0, 80]
#define WINDOW_SIZE         32    // ? Change the window size in range [8, 128]
#define ADAPTIVE            true  // ? Configurable Adaptive Timeout (change TIME_UNIT, MIN_TIMEOUT and MAX_TIMEOUT)
#define TIME_UNIT           4000  // ? 4 ms
#define MAX_TIMEOUT         80000 // ? 80 ms
#define MIN_TIMEOUT         8000  // ? 8 ms
#define MAX_ERRORS          25    // ? Maximum number of errors before closing the connection
#define TIMEOUT_CONNECTION  3600  // ? 1 hour
#define CONNECTION_ATTEMPTS 3     // ? Change the number of connection attempts
