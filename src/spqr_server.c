#include "spqr_server.h"

struct sockaddr_in server_addr;
socklen_t server_len = sizeof(server_addr);
struct sockaddr_in client_addr;
socklen_t client_len = sizeof(client_addr);
int32_t child_sockfd;

pid_t child_pids[MAX_CLIENTS];
uint8_t *shm_bitmask; // ? Shared memory area to store the bitmask of the clients connected.
int32_t shm_id;

char *server_pathname;

// ? This function is responsible for creating a socket and binding it to the port.
int32_t server_create_socket(const uint16_t port) {
	int32_t sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	spqr_assert(sockfd, "socket");

	memset(&server_addr, 0, server_len);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	spqr_assert(bind(sockfd, (struct sockaddr *) &server_addr, server_len), "bind");

#ifdef DEBUG
	printf("[DEBUG 🪲] Socket created successfully on port %d (sockfd: %d)\n", port, sockfd);
#endif

	return sockfd;
}

// ? This function is used to handle the signals from the parent process.
void parent_server_signal_handler(const int32_t signo) {
    if (signo == SIGINT || signo == SIGQUIT) {

	#ifdef DEBUG
		printf("\n[DEBUG 🪲] Server terminated due to %s.\n", signo == SIGINT ? "SIGINT" : "SIGQUIT");
	#endif

		for (uint32_t i = 0; i < MAX_CLIENTS; ++i) {
			if (child_pids[i] != INVALID_PID) {
				kill(child_pids[i], SIGTERM);
			}
		}

		for (uint32_t i = 0; i < MAX_CLIENTS; ++i) {
			if (child_pids[i] != INVALID_PID) {
				waitpid(child_pids[i], NULL, 0);
			#ifdef DEBUG
				printf("[DEBUG 🪲] Child process #%d terminated.\n", i);
			#endif
			}

			if (i == MAX_CLIENTS - 1) {
				spqr_assert(shmdt(shm_bitmask), "shmdt");
				spqr_assert(shmctl(shm_id, IPC_RMID, NULL), "shmctl");
			}
			
			child_pids[i] = INVALID_PID;
		}

		puts(SPQR_CLOSE_CONNECTION);
		exit(EXIT_SUCCESS);
    }
}

// ? This function is used to receive a packet.
char *server_receive_packet(char *msg, const int32_t sockfd, const uint64_t size) {
	memset(msg, 0, size);
	spqr_assert(recvfrom(sockfd, msg, size, MSG_WAITALL, (struct sockaddr *) &client_addr, &client_len), "recvfrom");
	return msg;
}

// ? This function is used to send a packet.
void server_send_packet(const char *msg, const int32_t sockfd) {
	spqr_assert(sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *) &client_addr, client_len), "sendto");
}

// ? This function is responsible for establishing the connection with the client.
// ? The logic of this function is based on the Three-way Handshake.
// ? The server receives a SYN packet from the client, then sends
// ? a SYNACK packet, then the server receives an ACK packet from the client.
bool server_open_connection_with_client(const int32_t sockfd) {
    char buffer[MAX_READ_LINE];
    memset(buffer, 0, MAX_READ_LINE);

    server_receive_packet(buffer, sockfd, MAX_READ_LINE);
    if (strcmp(buffer, udp.SYN)) {
	#ifdef DEBUG
        HANDLE_ERROR(udp.SYN);
    #endif
		return false;
    }

    server_send_packet(udp.SYNACK, sockfd);

    server_receive_packet(buffer, sockfd, MAX_READ_LINE);
    if (strcmp(buffer, udp.ACK)) {
    #ifdef DEBUG
		HANDLE_ERROR(udp.ACK);
    #endif
		return false;
    }

    return true;
}

// ? This function is used to set the bit of the bitmask.
void set_bit(const uint32_t client_id, const bitmask_t state) {
	int32_t byte_index = client_id / CHAR_BIT;
	int32_t bit_index = client_id % CHAR_BIT;
	if (state == BUSY) {
		shm_bitmask[byte_index] |= (1 << bit_index); // ? Set the bit to 1
	} else {
		shm_bitmask[byte_index] &= ~(1 << bit_index); // ? Set the bit to 0
	}
}

// ? This function is used to get the bit of the bitmask.
int8_t get_bit(const uint32_t client_id) {
	int32_t byte_index = client_id / CHAR_BIT;
	int32_t bit_index = client_id % CHAR_BIT;
	return (shm_bitmask[byte_index] >> bit_index) & 1;
}

// ? This function is used to handle the signals from the child process.
void child_server_signal_handler(const int32_t signo) {
	if (signo == SIGTERM) {
		char dummy[MAX_READ_LINE];
		server_receive_packet(dummy, child_sockfd, MAX_READ_LINE);
		memset(dummy, 0, MAX_READ_LINE);
		server_close_connection_with_client(child_sockfd);

		spqr_assert(close(child_sockfd), "close");

		exit(EXIT_SUCCESS);
	}
}

// ? This function is used to set up the parent and child signal handling.
void setup_signal_handling(signal_handler_t type) {
    struct sigaction sa;
    sigset_t set;

    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = 0;
    spqr_assert(sigemptyset(&sa.sa_mask), "sigemptyset");

    if (type == PARENT) {
        sa.sa_handler = parent_server_signal_handler;
        spqr_assert(sigaction(SIGINT, &sa, NULL), "sigaction");
        spqr_assert(sigaction(SIGQUIT, &sa, NULL), "sigaction");

    } else if (type == CHILD) {
        sa.sa_handler = child_server_signal_handler;
        spqr_assert(sigaction(SIGTERM, &sa, NULL), "sigaction");

        // ? Block all signals except SIGTERM
        spqr_assert(sigfillset(&set), "sigfillset");
        spqr_assert(sigdelset(&set, SIGTERM), "sigdelset");
    }

    spqr_assert(sigprocmask(SIG_BLOCK, &set, NULL), "sigprocmask");
}

// ? This function is used to list the files contained in a specific directory on the server.
bool get_server_files(const int32_t child_sockfd, const bool option, const char *filename) {
	DIR *dir;
	struct dirent *dp;
    
    char buffer[MAX_READ_LINE]; // ? Buffer to store the list of files in the directory
	memset(buffer, 0, MAX_READ_LINE);

	int32_t offset = 0;
	bool file_found = false;

	offset += snprintf(buffer, MAX_READ_LINE, "List of files availables on the server:\n");

	dir = opendir(SERVER_FILES_PATH);
	spqr_assert_ptr(dir, "opendir");
	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_type == DT_REG) {
			file_found = true;

			if (option && strcmp(filename, dp->d_name) == 0) {
				spqr_assert(closedir(dir), "closedir");
				return true;
			
			} else if (!option) {
				const char *emoji = get_file_emoji(dp->d_name);
				offset += snprintf(buffer + offset, MAX_READ_LINE - offset, "  %s %s\n", emoji, dp->d_name);

				if (offset >= MAX_READ_LINE - 1) {
					server_send_packet(buffer, child_sockfd);
					memset(buffer, 0, MAX_READ_LINE);
					offset = 0;
				}
			}
		}
	}

	if (!file_found) {
		memset(buffer, 0, MAX_READ_LINE);
		spqr_assert(snprintf(buffer, MAX_READ_LINE, "\n⚠️  Still no files for now.\n"), "snprintf");
	}

	if (!option && offset > 0) { server_send_packet(buffer, child_sockfd); }

	spqr_assert(closedir(dir), "closedir");
	return false;
}

// ? This function is used to close the connection with the client.
// ? The logic of this function is based on the Three-way Handshake.
// ? The server receives a FIN packet from the client, then sends
// ? a FINACK packet, then the server sends a FIN packet to the client
// ? and the client responds with a FINACK packet.
int8_t server_close_connection_with_client(const int32_t sockfd) {
	char buffer[MAX_READ_LINE];
    memset(buffer, 0, MAX_READ_LINE);

	server_receive_packet(buffer, sockfd, MAX_READ_LINE);
	if (strcmp(buffer, udp.FIN)) {
	#ifdef DEBUG
		HANDLE_ERROR(udp.FIN);
		printf("\n\n%s", buffer);
	#endif
		return TRANSFER_ERROR;
	}
	
	server_send_packet(udp.FINACK, sockfd);
	server_send_packet(udp.FIN, sockfd);
	
	server_receive_packet(buffer, sockfd, MAX_READ_LINE);
	if (strcmp(buffer, udp.FINACK)) {
	#ifdef DEBUG
        HANDLE_ERROR("FINACK");
	#endif
		return TRANSFER_ERROR;
	}

	return EXIT_SUCCESS;
}

// ? This function is used to manage the commands received from the client.
int8_t server_manage_client_commands(const int32_t child_sockfd) {
	int8_t ret;
    char request[MAX_READ_LINE]; // ? Buffer to store `LIST`, `GET`, `PUT` and `CLOSE` commands.
    
    while (true) {
    start:
		server_receive_packet(request, child_sockfd, MAX_READ_LINE);
		spqr_upper(request);

		if (strcmp(request, cmd.LIST) == 0) {
			get_server_files(child_sockfd, false, "");
		
        } else if (strcmp(request, cmd.GET) == 0) {
			memset(request, 0, MAX_READ_LINE);
			server_send_packet(SPQR_INSERT_FILENAME, child_sockfd);

			char filename[MAX_READ_LINE];
			server_receive_packet(filename, child_sockfd, MAX_READ_LINE);
			if (get_server_files(child_sockfd, true, filename)) {
				server_send_packet(filename, child_sockfd);
			} else {
				server_send_packet(SPQR_FILENAME_NOT_FOUND, child_sockfd);
				goto start;
			}

			uint64_t path_size = strlen(SERVER_FILES_PATH) + strlen(filename) + 1;
			server_pathname = malloc(path_size);
			spqr_assert_ptr(server_pathname, "malloc");
			spqr_assert(snprintf(server_pathname, path_size, "%s%s", SERVER_FILES_PATH, filename), "snprintf");

			set_timeout(child_sockfd, MAX_TIMEOUT); // ? Set the timeout of the socket.
			
			ret = TRANSFER_ERROR;

			server_receive_packet(request, child_sockfd, MAX_READ_LINE);
			if (strcmp(request, feedback.START_TRANSFER) == 0) {
				set_timeout(child_sockfd, 0); // ? Reset the timeout of the socket
				ret = main_sender(child_sockfd, &client_addr, server_pathname);
			}
			set_timeout(child_sockfd, 0); // ? Reset the timeout of the socket
			spqr_free(&server_pathname);

			if (ret == EXIT_SUCCESS) {
            #ifdef DEBUG
                printf("[DEBUG 🪲] Transfer completed successfully.\n");
            #endif
            
			    server_send_packet(feedback.TRANSFER_COMPLETED, child_sockfd);
			
			} else if (ret == TRANSFER_ERROR) {
            #ifdef DEBUG
                printf("[DEBUG 🪲] Transfer failed with error code: %d\n", ret);
            #endif

				server_send_packet(feedback.TRANSFER_ERROR, child_sockfd);
			}
		
        } else if (strcmp(request, cmd.PUT) == 0) {
			server_send_packet(cmd.PUT, child_sockfd);
			memset(request, 0, MAX_READ_LINE);
			
			server_receive_packet(request, child_sockfd, MAX_READ_LINE);
			if (strcmp(request, feedback.ABORT) == 0) { goto start; }

			if (get_server_files(child_sockfd, true, request)) {
				server_send_packet(file.EXIST, child_sockfd);
				goto start;
			}
			server_send_packet(file.NOT_EXIST, child_sockfd);
			
            uint64_t path_size = strlen(SERVER_FILES_PATH) + strlen(request) + 1;
			server_pathname = malloc(path_size);
			spqr_assert_ptr(server_pathname, "malloc");
			snprintf(server_pathname, path_size, "%s%s", SERVER_FILES_PATH, request);
			ret = main_receiver(child_sockfd, &client_addr, server_pathname);
			if (ret == TRANSFER_ERROR) {
			#ifdef DEBUG
				printf("[DEBUG 🪲] Transfer failed with error code: %d\n", ret);
				printf("[DEBUG 🪲] The file \"%s\" has not been received.\n", request);
			#endif
			
				memset(request, 0, MAX_READ_LINE);
				spqr_free(&server_pathname);
				goto start;
			}

			// ? Wait for the client to send the SPQR_TRANSFER_COMPLETED message
			memset(server_pathname, 0, path_size);
			while (true) {
				set_timeout(child_sockfd, TIMEOUT);
				recvfrom(child_sockfd, server_pathname, path_size, 0, (struct sockaddr *) &client_addr, &client_len);
				if (strcmp(server_pathname, feedback.TRANSFER_COMPLETED) == 0) {
					set_timeout(child_sockfd, 0);
					break;
				}
			}

			spqr_free(&server_pathname);
		
        } else if (strcmp(request, cmd.CLOSE) == 0) {
			server_send_packet(cmd.CLOSE, child_sockfd);

			// printf("Closed connection for client #%d\n", no_client);

			ret = server_close_connection_with_client(child_sockfd);

			close(child_sockfd);
			return ret; // ? the function `server_manage_client_commands` returns 0 in case of correct closure

		} else if (strcmp(request, feedback.EXIT) == 0) {
			// printf("Closed connection for client #%d\n", no_client);

			ret = server_close_connection_with_client(child_sockfd);

			close(child_sockfd);
			return ret;
		}
	#ifdef RICK_ROLL
		else if (strcmp(request, cmd.RICK) == 0) {
			server_send_packet(cmd.RICK, child_sockfd);
		} 
	#endif
		else {
			server_send_packet(SPQR_COMMAND_NOT_FOUND, child_sockfd);
		}
	}
}

// ? This function is used to convert a string to a 32-bit integer.
SPQR(int32_t, to_int32, strtol, 10)

// ? This function is responsible for managing the connection with the client.
int8_t server_manage_client_connections(const int32_t sockfd) {
    uint16_t port;
    char buffer[MAX_READ_LINE];
    memset(buffer, 0, sizeof(buffer));

    // ? Initialize shared memory and bitmask as before
    key_t key = ftok(KEY_PATH, 's');
    spqr_assert(key, "ftok");
    
    shm_id = shmget(key, SHM_SIZE, IPC_CREAT | PERMS);
    spqr_assert(shm_id, "shmget");
    
    shm_bitmask = (uint8_t *)shmat(shm_id, NULL, 0);
    spqr_assert_shm(shm_bitmask, "shmat");
    memset(shm_bitmask, FREE, SHM_SIZE);
    
    memset(child_pids, INVALID_PID, sizeof(child_pids));

    // ? Set up the file descriptor set.
    fd_set read_fds, master_fds;
    int32_t max_fd = sockfd;
    
    FD_ZERO(&master_fds);
    FD_SET(sockfd, &master_fds);

    // ? Main server loop.
    while (true) {
        read_fds = master_fds;
        
        // ? Set timeout for select
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int32_t ready = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ready < 0) {
            if (errno == EINTR) continue; // ? Handle interruption by signal.
            HANDLE_ERROR("select");
            break;
        }

        // ? Check for new connections on the main socket.
        if (FD_ISSET(sockfd, &read_fds)) {
            if (server_open_connection_with_client(sockfd)) {
                uint32_t no_client = 0;
                for (; no_client < MAX_CLIENTS; ++no_client) {
                    if (!get_bit(no_client)) {
                        set_bit(no_client, BUSY);
                        break;
                    }
                    
                    if (no_client == MAX_CLIENTS - 1) {
                        server_send_packet(SPQR_SERVER_BUSY, sockfd);
                        continue;
                    }
                }

                port = PORT + no_client;
                memset(buffer, 0, sizeof(buffer));
                spqr_assert(snprintf(buffer, sizeof(buffer), "%d", port), "snprintf");
                server_send_packet(buffer, sockfd);
                printf("\nThe client #%d is connected on port %d.\n", no_client, port);

                pid_t pid = fork();
                spqr_assert(pid, "fork");
                
                if (pid == 0) { // ? Child process.
                    spqr_assert(close(sockfd), "close"); // ? Close the parent's socket because it is not needed.
                    setup_signal_handling(CHILD);
                    init_packet_loss_simulator();
                    
                    child_sockfd = server_create_socket(port);
                    int32_t status = server_manage_client_commands(child_sockfd);
                    
                    set_bit(no_client, FREE);
                    printf("Closed connection for client #%d on port %d with exit code %d.\n", no_client, port, status);
                    
                    exit(status); // ? Close the child process.

                } else { // ? Parent process.
                    child_pids[no_client] = pid;
                }
            }
        }

        // ? Check for zombie processes and clean them up.
        int32_t status;
        pid_t wpid;
        while ((wpid = waitpid(INVALID_PID, &status, WNOHANG)) > 0) {
            // ? Handle terminated child process.
            for (uint32_t i = 0; i < MAX_CLIENTS; ++i) {
                if (child_pids[i] == wpid) {
                    child_pids[i] = INVALID_PID;
                    set_bit(i, FREE);
                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
