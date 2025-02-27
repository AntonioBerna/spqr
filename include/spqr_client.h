#pragma once

#include "stdc.h"

// * Prototypes and variables * //
extern struct sockaddr_in server_addr;
extern socklen_t server_len;
extern int32_t sockfd;
extern char *server_response;
extern char *client_pathname;

void client_send_packet(const char *msg, const int32_t sockfd);
char *client_receive_packet(char *msg, const int32_t sockfd, const uint64_t size);
int8_t client_close_connection_with_server(const int32_t sockfd);
void client_signal_handler(const int32_t signo);
void setup_signal_handling(void);
int32_t client_create_socket(const char *ip, const uint16_t port);
bool client_open_connection_with_server(const int32_t sockfd);
bool client_establish_connection_with_server(const char *ip);
void get_user_input(const char *msg, char *request, const int32_t sockfd, const bool flag);
void show_list_of_client_files(void);
void get_filename_user_input(char *filename);
bool filename_cmp(const char *filename);
int8_t client_manage_server_commands(void);
