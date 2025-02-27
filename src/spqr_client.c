#include "spqr_client.h"

struct sockaddr_in server_addr;
socklen_t server_len = sizeof(server_addr);
int32_t sockfd;
char *server_response;
char *client_pathname;

// ? This function is used to send a packet.
void client_send_packet(const char *msg, const int32_t sockfd) {
	spqr_assert(sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *) &server_addr, server_len), "sendto");
}

// ? This function is used to receive a packet.
char *client_receive_packet(char *msg, const int32_t sockfd, const uint64_t size) {
	memset(msg, 0, size);
	int64_t n = recvfrom(sockfd, msg, size, MSG_WAITALL, (struct sockaddr *) &server_addr, &server_len);
	if (n < 0) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			return NULL;

		} else {
		#ifdef DEBUG
			HANDLE_ERROR("recvfrom");
		#endif
		}
	}
	return msg;
}

// ? This function is used to close the connection with the server.
// ? The logic of this function is based on the Three-way Handshake.
// ? The client sends a FIN packet to the server, the server responds
// ? with a FINACK packet, then the server sends a FIN packet to the client
// ? and the client responds with a FINACK packet.
int8_t client_close_connection_with_server(const int32_t sockfd) {
	char buffer[MAX_READ_LINE];
	memset(buffer, 0, MAX_READ_LINE);

	client_send_packet(udp.FIN, sockfd);

	client_receive_packet(buffer, sockfd, MAX_READ_LINE);
	if (strcmp(buffer, udp.FINACK)) {
	#ifdef DEBUG
		HANDLE_ERROR(udp.FINACK);
	#endif
		return TRANSFER_ERROR;
	}

	client_receive_packet(buffer, sockfd, MAX_READ_LINE);
	if (strcmp(buffer, udp.FIN)) {
	#ifdef DEBUG
		HANDLE_ERROR(udp.FIN);
	#endif
		return TRANSFER_ERROR;
	}
	
	client_send_packet(udp.FINACK, sockfd);

	puts(SPQR_CLOSE_CONNECTION);

	return EXIT_SUCCESS;
}

// ? This function is used to handle the signals.
void client_signal_handler(const int32_t sig) {
	if (sig == SIGINT) {
	#ifdef DEBUG
		puts("\n" SPQR_SESSION_STOPPED);
	#endif
		client_send_packet(feedback.EXIT, sockfd);
		client_close_connection_with_server(sockfd);

	} else if (sig == SIGQUIT) {
	#ifdef DEBUG
		puts("\n" SPQR_SESSION_STOPPED);
	#endif
		client_send_packet(feedback.EXIT, sockfd);

	} else if (sig == SIGALRM) {
		puts("\n" SPQR_TIMEOUT_SESSION);

		client_send_packet(feedback.EXIT, sockfd);
		client_close_connection_with_server(sockfd);
	}

	spqr_free(&server_response);
	spqr_free(&client_pathname);
	spqr_assert(close(sockfd), "close");

	exit(EXIT_SUCCESS);
}

// ? This function is used to set up the signal handling.
void setup_signal_handling(void) {
	struct sigaction sa;
	sigset_t set;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = client_signal_handler;
    sa.sa_flags = 0;
	spqr_assert(sigemptyset(&sa.sa_mask), "sigemptyset");
	spqr_assert(sigaction(SIGINT, &sa, NULL), "sigaction");
	spqr_assert(sigaction(SIGQUIT, &sa, NULL), "sigaction");
	spqr_assert(sigaction(SIGALRM, &sa, NULL), "sigaction");
	
	// ? Block all signals except SIGINT, SIGQUIT, SIGALRM.
	spqr_assert(sigfillset(&set), "sigfillset");
	spqr_assert(sigdelset(&set, SIGINT), "sigdelset");
	spqr_assert(sigdelset(&set, SIGQUIT), "sigdelset");
	spqr_assert(sigdelset(&set, SIGALRM), "sigdelset");
	spqr_assert(sigprocmask(SIG_BLOCK, &set, NULL), "sigprocmask");
}

// ? This function is used to convert a string to a 16-bit unsigned integer.
SPQR(uint16_t, to_uint16, strtoul, 10)

// ? This function is used to create a socket.
int32_t client_create_socket(const char *ip, const uint16_t port) {
	int32_t sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	spqr_assert(sockfd, "socket");

	memset(&server_addr, 0, server_len);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	int32_t result = inet_pton(AF_INET, ip, &(server_addr.sin_addr));
	if (result <= 0) {
	#ifdef DEBUG
		HANDLE_ERROR("inet_pton");
	#endif
		return -1;
	}

	return sockfd;
}

// ? This function is responsible for managing the connection with the server.
// ? The logic of this function is based on the Three-way Handshake.
// ? The client sends a SYN packet to the server, the server responds
// ? with a SYNACK packet, then the server sends an ACK packet to the client.
bool client_open_connection_with_server(const int32_t sockfd) {
	char buffer[MAX_READ_LINE];
	memset(buffer, 0, MAX_READ_LINE);

	client_send_packet(udp.SYN, sockfd);

	client_receive_packet(buffer, sockfd, MAX_READ_LINE);
	if (strcmp(buffer, udp.SYNACK)) {
	#ifdef DEBUG
		HANDLE_ERROR(udp.SYNACK);
	#endif
		return false;
	}

	client_send_packet(udp.ACK, sockfd);
	
	return true;
}

// ? This function is used to establish the connection with the server.
bool client_establish_connection_with_server(const char *ip) {
	int8_t connection_attempts = CONNECTION_ATTEMPTS;

	sockfd = client_create_socket(ip, PORT - 1);
	spqr_assert(sockfd, "client_create_socket");

	set_seconds_timeout(sockfd, CONNECTION_ATTEMPTS);
	while (connection_attempts > 0) {
		if (!client_open_connection_with_server(sockfd)) {
			printf(COLOR_RED "⚠️  Connection refused (remaining attempts: %d)\n", --connection_attempts);
			continue;
		}
		set_seconds_timeout(sockfd, 0);
		
		client_receive_packet(server_response, sockfd, MAX_READ_LINE);
		if (strcmp(server_response, SPQR_SERVER_BUSY) == 0) {
			puts(SPQR_SERVER_BUSY);
			return false;
		}
		uint16_t port;
		if (!to_uint16(server_response, &port)) {
		#ifdef DEBUG	
			HANDLE_ERROR("to_uint16");
		#endif
			return false;
		}

		spqr_assert(close(sockfd), "close");
		sockfd = client_create_socket(ip, port);
		break;
	}
	set_seconds_timeout(sockfd, 0);

	if (connection_attempts == 0) {
		puts(COLOR_RESET SPQR_CONNECTION_REFUSED);
		return false;
	}

	puts(COLOR_RESET);
	return true;
}

// ? This function is used to get the user input.
void get_user_input(const char *msg, char *request, const int32_t sockfd, const bool flag) {
	if (flag) { puts(LEGEND); }
	
	while (true) {
		if (flag) { printf("%s", msg); }
		memset(request, 0, MAX_READ_LINE);
		if (fgets(request, MAX_READ_LINE, stdin) == NULL) {
			puts(SPQR_INVALID_INPUT);
			continue;
		}
		request[strcspn(request, "\n")] = '\0';
		break;
	}

	client_send_packet(request, sockfd);
	set_seconds_timeout(sockfd, 1);
	void *ret = client_receive_packet(server_response, sockfd, MAX_READ_LINE);
	if (ret == NULL) {
		set_seconds_timeout(sockfd, 0);
		spqr_assert(snprintf(request, MAX_READ_LINE, "%s", feedback.EXIT), "snprintf");
		return;
	}
	set_seconds_timeout(sockfd, 0);
	if (strcmp(server_response, cmd.CLOSE) && strcmp(server_response, cmd.GET) && strcmp(server_response, cmd.PUT)) {
	#ifdef RICK_ROLL
		if (strcmp(server_response, cmd.RICK)) {
			if (flag) { printf("\n%s", server_response); }
		}
	#else
		if (flag) { printf("\n%s", server_response); }
	#endif
	}

	if (flag) { spqr_upper(request); }
}

// ? This function is used to show the list of files in the client directory.
void show_list_of_client_files(void) {
	DIR *dir = opendir(CLIENT_FILES_PATH);
	spqr_assert_ptr(dir, "opendir");

	printf("\nList of files in the %s directory:\n", CLIENT_FILES_PATH);
	struct dirent *dp;
	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_type == DT_REG) {
			const char *emoji = get_file_emoji(dp->d_name);
			printf("  %s %s\n", emoji, dp->d_name);
		}
	}
	spqr_assert(closedir(dir), "closedir");
}

// ? This function is used to get the filename from the user input.
void get_filename_user_input(char *filename) {
	while (true) {
		printf("\n%s", SPQR_INSERT_FILENAME);
		memset(filename, 0, MAX_READ_LINE);
		if (fgets(filename, MAX_READ_LINE, stdin) == NULL) {
			puts(SPQR_INVALID_INPUT);
			continue;
		}
		filename[strcspn(filename, "\n")] = '\0';
		break;
	}
}

// ? This function is used to compare a filename with the filenames of the files in the directory.
bool filename_cmp(const char *filename) {
	DIR *dir = opendir(CLIENT_FILES_PATH);
	spqr_assert_ptr(dir, "opendir");
	
	struct dirent *dp;
	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_type == DT_REG) {
			if (strcmp(filename, dp->d_name) == 0) {
				spqr_assert(closedir(dir), "closedir");
				return true;
			}
		}
	}

	spqr_assert(closedir(dir), "closedir");
	return false;
}

// ? This function is used to manage the commands received from the server.
int8_t client_manage_server_commands(void) {
	char request[MAX_READ_LINE]; // ? Buffer to store `LIST`, `GET`, `PUT` and `CLOSE` commands.
    memset(request, 0, MAX_READ_LINE);

	int8_t ret;
	while (true) {
	start:
		alarm(TIMEOUT_CONNECTION); // ? Set the alarm.

		if (strcmp(request, cmd.CLOSE) == 0) {
			ret = client_close_connection_with_server(sockfd);
			if (ret == TRANSFER_ERROR) {
				HANDLE_ERROR(SPQR_ERROR_CLOSE_CONNECTION);
				goto close;
			}
			break;

		} else if (strcmp(request, cmd.GET) == 0) {
			memset(server_response, 0, MAX_READ_LINE);
			get_user_input("", request, sockfd, false);
			if (strcmp(server_response, SPQR_FILENAME_NOT_FOUND) == 0) {
				puts("\n" SPQR_FILENAME_NOT_FOUND);
				memset(request, 0, MAX_READ_LINE);
				goto start;
			}
			
			uint64_t path_size = strlen(CLIENT_FILES_PATH) + strlen(server_response) + 1;
			client_pathname = malloc(path_size);
			spqr_assert_ptr(client_pathname, "malloc");
			memset(client_pathname, 0, path_size);
			spqr_assert(snprintf(client_pathname, path_size, "%s%s", CLIENT_FILES_PATH, server_response), "snprintf");
			ret = main_receiver(sockfd, &server_addr, client_pathname);

			spqr_free(&client_pathname);
			memset(server_response, 0, MAX_READ_LINE);

			// ? This `while` loop has the sole purpose of consuming all the duplicate packets
			// ? sent by the sender due to any lost acks or interrupted timeouts.
			while (true) {
				set_timeout(sockfd, TIMEOUT);
				if (ret == EXIT_SUCCESS) { recvfrom(sockfd, server_response, strlen(feedback.TRANSFER_COMPLETED), 0, (struct sockaddr *) &server_addr, &server_len); }
				if (ret == TRANSFER_ERROR) { recvfrom(sockfd, server_response, strlen(feedback.TRANSFER_ERROR), 0, (struct sockaddr *) &server_addr, &server_len); }
				if (strcmp(server_response, feedback.TRANSFER_COMPLETED) == 0 || strcmp(server_response, feedback.TRANSFER_ERROR) == 0) {
					set_timeout(sockfd, 0);
					break;
				}
			}
		
		} else if (strcmp(request, cmd.PUT) == 0) {
			memset(request, 0, MAX_READ_LINE);

			show_list_of_client_files();

			char filename[MAX_READ_LINE];
			get_filename_user_input(filename);
			if (filename_cmp(filename)) {
				client_send_packet(filename, sockfd);
				client_receive_packet(request, sockfd, MAX_READ_LINE);
				if (strcmp(request, file.EXIST) == 0) {
					memset(request, 0, MAX_READ_LINE);
					puts(SPQR_FILE_EXIST_ON_SERVER);
					goto start;
				}
			} else {
				printf("❓ File \"%s\" not found.\n", filename);
				client_send_packet(feedback.ABORT, sockfd);
				memset(filename, 0, MAX_READ_LINE);
				goto start;
			}

			uint64_t path_size = strlen(CLIENT_FILES_PATH) + strlen(filename) + 1;
			client_pathname = malloc(path_size);
			spqr_assert_ptr(client_pathname, "malloc");
			memset(client_pathname, 0, path_size);
			spqr_assert(snprintf(client_pathname, path_size, "%s%s", CLIENT_FILES_PATH, filename), "snprintf");

			memset(server_response, 0, MAX_READ_LINE);
			client_receive_packet(server_response, sockfd, MAX_READ_LINE);
			if (strcmp(server_response, feedback.START_TRANSFER) == 0) {
				ret = main_sender(sockfd, &server_addr, client_pathname);
				if (ret == TRANSFER_ERROR) {
					spqr_free(&client_pathname);
					goto start;
				}
			}

			client_send_packet(feedback.TRANSFER_COMPLETED, sockfd);

			spqr_free(&client_pathname);
			
		}
	#ifdef RICK_ROLL
		else if (strcmp(request, cmd.RICK) == 0) {

			#ifdef __linux__
				// ! char *amixer_cmd = "/usr/bin/amixer";
				// ! char *amixer_args[] = { "amixer", "set", "Master", "100%", NULL };
				// ! execute_command(amixer_cmd, amixer_args);

				char *mpv_cmd = "/usr/bin/mpv";
				char *mpv_args[] = { "mpv", "assets/video/Kbsbo Dlkkx Dfsb Vlr Rm.mp4", NULL };
				execute_command(mpv_cmd, mpv_args);
			
			#elif __APPLE__
				// ! char *osascript_cmd = "/usr/bin/osascript";
				// ! char *osascript_args[] = { "osascript", "-e", "set volume output volume 100", NULL };
				// ! execute_command(osascript_cmd, osascript_args);

				char *open_cmd = "/usr/bin/open";
				char *open_args[] = { "open", "-a", "VLC", "assets/video/Kbsbo Dlkkx Dfsb Vlr Rm.mp4", NULL };
				execute_command(open_cmd, open_args);
			#endif

			memset(request, 0, MAX_READ_LINE);
		}
	#endif
		else { // ? `LIST` command.
			memset(server_response, 0, MAX_READ_LINE);
			get_user_input(CLIENT_PROMPT, request, sockfd, true);
			if (strcmp(request, feedback.EXIT) == 0) {
				puts(SPQR_SESSION_TERMINATED);
				client_close_connection_with_server(sockfd);
				break;
			}
		}

		alarm(0); // ? Reset the alarm.
	}

	ret = EXIT_SUCCESS;

close:
	spqr_free(&server_response);
	spqr_free(&client_pathname);
	spqr_assert(close(sockfd), "close");

	return ret;
}
