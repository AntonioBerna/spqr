#pragma once

#include "stdc.h"

// * Three-way Handshake * //
typedef struct {
    const char *SYN;
    const char *SYNACK;
    const char *ACK;
    const char *FIN;
    const char *FINACK;
} three_way_handshake_t;

extern const three_way_handshake_t udp;

// * Commands * //
typedef struct {
    const char *LIST;
    const char *GET;
    const char *PUT;
    const char *CLOSE;
#ifdef RICK_ROLL
    const char *RICK;
#endif
} cmd_t;

extern const cmd_t cmd;

// * File management * //
typedef struct {
    const char *EXIST;
    const char *NOT_EXIST;
} file_t;

extern const file_t file;

// * Feedback * //
typedef struct {
    const char *START_TRANSFER;
    const char *TRANSFER_COMPLETED;
    const char *TRANSFER_ERROR;
    const char *EXIT;
    const char *ABORT;
} feedback_t;

extern const feedback_t feedback;

// * Debug messages * //
#ifdef DEBUG
#define SPQR_FILENAME_SUCCESSFULLY_DELETED "[DEBUG 🪲] 🗑️ The file is deleted successfully."
#define SPQR_SESSION_STOPPED               "[DEBUG 🪲] 🚫 Session stopped by client."
#endif

// * General messages * //
#define SPQR_SERVER_BUSY                   "⏳ The server is completely busy."
#define SPQR_INSERT_FILENAME               "📂 Insert file to transfer: "
#define SPQR_FILENAME_NOT_FOUND            "❓ Filename not found."
#define SPQR_COMMAND_NOT_FOUND             "⚠️ Command not found, please retry.\n"
#define SPQR_SESSION_TERMINATED            "⛔️ Session terminated by server."
#define SPQR_TIMEOUT_SESSION               "⏲️ Timeout session."
#define SPQR_CONNECTION_REFUSED            "⛔️ Connection refused. Closing the client."
#define SPQR_INVALID_INPUT                 "🚫 Invalid input. Try again."
#define SPQR_ERROR_CLOSE_CONNECTION        "⚠️ Error closing connection with server."
#define SPQR_CLOSE_CONNECTION              "\nBye bye! 👋"
#define SPQR_FILE_EXIST_ON_SERVER          "📁 File exists on the server."

// * Colors * //
#define COLOR_RESET  "\033[0m"
#define COLOR_RED    "\033[31m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_YELLOW "\033[33m"

// * Common Definitions * //
#define LEGEND \
    COLOR_RESET "\n" \
    "╔═══════════╦══════════════════════════════════════════╗\n" \
    "║  Command  ║                Description               ║\n" \
    "╠═══════════╬══════════════════════════════════════════╣\n" \
    "║ 🗂️  LIST   ║ List the files available on the server   ║\n" \
    "╠═══════════╬══════════════════════════════════════════╣\n" \
    "║ 📥 GET    ║ Download a file from the server          ║\n" \
    "╠═══════════╬══════════════════════════════════════════╣\n" \
    "║ 📤 PUT    ║ Upload a file to the server              ║\n" \
    "╠═══════════╬══════════════════════════════════════════╣\n" \
    "║ ❌ CLOSE  ║ Close the connection                     ║\n" \
    "╚═══════════╩══════════════════════════════════════════╝\n" \

#define INFO \
    COLOR_GREEN \
    "\n" \
    "╔══════════════════════════╗\n" \
    "║ CONFIGURATION PARAMETERS ║\n" \
    "╠══════════════════════════╣\n" \
    "║ WINDOW_SIZE      = %d    ║\n" \
    "║ LOSS_PROBABILITY = %d%%   ║\n" \
    "║ TIMEOUT          = %d  ║\n" \
    "║ ADAPTIVE         = %s  ║\n" \
    "╚══════════════════════════╝\n" \
    COLOR_RESET ""

#ifdef IS_CLIENT
#define FILE_TRANSFER_COMPLETED \
    COLOR_GREEN \
    "╔═════════════════════════════════╗\n" \
    "║ FILE TRANSFER COMPLETED         ║\n" \
    "╠═════════════════════════════════╣\n" \
    "║ TIME       = %.6f sec       ║\n" \
    "║ THROUGHPUT = %.2f KB/s      ║\n" \
    "╚═════════════════════════════════╝\n" \
    COLOR_RESET ""

#define FILE_TRANSFER_FAILED \
    COLOR_RED \
    "╔═════════════════════════════════╗\n" \
    "║ FILE TRANSFER FAILED            ║\n" \
    "╠═════════════════════════════════╣\n" \
    "║ TIME       = %.6f sec       ║\n" \
    "║ THROUGHPUT = %.2f KB/s      ║\n" \
    "╚═════════════════════════════════╝\n" \
    COLOR_RESET ""

#define SPQR_KB 1024
#endif

#define DIE(msg) \
	do { \
		perror(msg); \
		exit(EXIT_FAILURE); \
	} while (0)

#define HANDLE_ERROR(msg) \
	do { \
		fprintf(stderr, COLOR_RESET "Error in file %s, line %d, function %s: %s - %s\n", __FILE__, __LINE__, __func__, msg, strerror(errno)); \
	} while (0)

#define MAX_READ_LINE 1024

// * ASCII Prototypes * //
void load_ascii_art(void);
void print_progress(const uint32_t total, const uint32_t current);
const char *get_file_emoji(const char *filename);

// * Timeouts Prototypes * //
void init_packet_loss_simulator(void);
bool can_send_packet(void);
suseconds_t get_timeout(const int32_t sockfd);
void set_timeout(const int32_t sockfd, const suseconds_t timeout);
void set_seconds_timeout(const int32_t sockfd, const suseconds_t timeout);
void increase_timeout(const int32_t sockfd);
void decrease_timeout(const int32_t sockfd);

// * SPQR Prototypes * //
#define SPQR(TYPE, FUNC_NAME, STR_TO_FUNC, BASE) \
bool FUNC_NAME(const char *buffer, TYPE *value) { \
    char *endptr; \
    errno = 0; \
    *value = (TYPE)STR_TO_FUNC(buffer, &endptr, BASE); \
    return !(errno == ERANGE || endptr == buffer || (*endptr && *endptr != '\n')); \
}

void spqr_assert(const int64_t ret, const char *msg);
void spqr_assert_ptr(const void *ptr, const char *msg);
void spqr_assert_shm(const void *addr, const char *msg);
void spqr_free(char **ptr);
void spqr_upper(char *msg);

// * Rick Roll Prototypes * //
#ifdef RICK_ROLL
extern char *environ;
void execute_command(const char *command, char *const *argv);
#endif
