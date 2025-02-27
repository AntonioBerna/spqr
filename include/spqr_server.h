#pragma once

#include "stdc.h"

extern struct sockaddr_in server_addr;
extern socklen_t server_len;
extern struct sockaddr_in client_addr;
extern socklen_t client_len;
extern int32_t child_sockfd;

typedef enum {
    FREE = 0x00,
    BUSY = 0xFF
} bitmask_t;

typedef enum {
    PARENT,
    CHILD
} signal_handler_t;

// ? Each clients require only one bit (0 or 1) and for manage 2727 clients I need to use 2727 bit.
// ? If I convert 2727 bit in bytes I obtain 2727 / 8 = 340.875 ~= 341 bytes.
#define SHM_SIZE ((MAX_CLIENTS + 7) / CHAR_BIT) // ? 341 bytes
#define KEY_PATH ASCII_ART_PATH

#ifdef DEBUG
#define PERMS 0777
#else
#define PERMS 0600
#endif

#define INVALID_PID -1

extern pid_t child_pids[MAX_CLIENTS];
extern uint8_t *shm_bitmask; // ? Shared memory area to store the bitmask of the clients connected.
extern int32_t shm_id;

extern char *server_pathname;

// * Prototypes * //
int32_t server_create_socket(const uint16_t port);
void parent_server_signal_handler(const int32_t signo);
char *server_receive_packet(char *msg, const int32_t sockfd, const uint64_t size);
void server_send_packet(const char *msg, const int32_t sockfd);
bool server_open_connection_with_client(const int32_t sockfd);
void set_bit(const uint32_t client_id, const bitmask_t state);
int8_t get_bit(const uint32_t client_id);
void child_server_signal_handler(const int32_t signo);
void setup_signal_handling(signal_handler_t type);
bool get_server_files(const int32_t child_sockfd, const bool option, const char *filename);
int8_t server_close_connection_with_client(const int32_t sockfd);
int8_t server_manage_client_commands(const int32_t child_sockfd);
int8_t server_manage_client_connections(const int32_t sockfd);
