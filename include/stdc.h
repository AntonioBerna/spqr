#pragma once

// * Networking libraries * //
#include <arpa/inet.h>

// * Error handling and standard I/O * //
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// * Thread, Process and synchronization * //
#ifdef RICK_ROLL
#include <spawn.h>
#endif
#include <sys/wait.h>

// * File operations and system calls * //
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

// * Additional libraries * //
#include <ctype.h>
#include <sys/time.h>
#include <time.h>

// * POSIX and Miscellaneous definitions * //
#ifdef __linux__
#define __USE_POSIX 1
#endif
#include <signal.h>
#include <stdio.h>

#ifdef __linux__
#define __USE_MISC 1
#endif
#include <dirent.h>
#include <unistd.h>

// * Custom headers * //
#include "ascii.h"
#include "common.h"
#include "messages.h"
#include "settings.h"
#include "protocol.h"
