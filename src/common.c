#include "common.h"

const three_way_handshake_t udp = {
	.SYN = "SYN",
	.SYNACK = "SYNACK",
	.ACK = "ACK",
	.FIN = "FIN",
	.FINACK = "FINACK",
};

const cmd_t cmd = {
    .LIST = "LIST",
    .GET = "GET",
    .PUT = "PUT",
    .CLOSE = "CLOSE",
#ifdef RICK_ROLL
    .RICK = "RICK",
#endif
};

const file_t file = {
	.EXIST = "EXIST",
	.NOT_EXIST = "NOT_EXIST",
};

const feedback_t feedback = {
	.START_TRANSFER = "START_TRANSFER",
	.TRANSFER_COMPLETED = "TRANSFER_COMPLETED",
	.TRANSFER_ERROR = "TRANSFER_ERROR",
	.EXIT = "EXIT",
	.ABORT = "ABORT",
};

// ? This function reads the ASCII art from the file and prints it to the console.
void load_ascii_art(void) {
    FILE *ascii_art_file = fopen(ASCII_ART_PATH, "r");
    spqr_assert_ptr(ascii_art_file, "fopen");

    char buffer[MAX_READ_LINE];
    while (fgets(buffer, sizeof(buffer), ascii_art_file) != NULL) {
        printf("%s", buffer);
    }
    printf("\n\n\tDeveloped by %s\n\n", AUTHORS);

    spqr_assert(fclose(ascii_art_file), "fclose");
}

// ? This function is used to print the progress of the file transfer.
// ? The percentage of progress is calculated using the formula:
// ?     progress = (current / total) * 100
void print_progress(const uint32_t total, const uint32_t current) {
    // Ensure we can reach 100% by handling edge cases
    double progress;
    
    if (total == 0) {
        progress = 100.0; // Avoid division by zero
    } else if (current >= total) {
        progress = 100.0; // Force 100% when we've reached or exceeded total
    } else {
        progress = (double)current / total * 100.0;
    }
    
    // ? Each emoji represents 5%
    uint32_t no_emoji;
    if (progress >= 99.9) {
        // Force full bar at the end
        no_emoji = 20;
        progress = 100.0;
    } else {
        no_emoji = (uint32_t)(progress / 5.0);
    }
    
    printf("\r[%.1f%%] [", progress);
    for (uint32_t i = 0; i < 20; ++i) {
        if (i < no_emoji) {
            printf("%s", PACKAGE_EMOJI);
        } else {
            printf(" ");
        }
    }
    printf("]");
    spqr_assert(fflush(stdout), "fflush");
}

// ? This function is used to get the emoji associated with the file.
const char *get_file_emoji(const char *filename) {
	const char *ext = strrchr(filename, '.');
	if (!ext) return "❓";
	if (strcmp(ext, ".txt") == 0) return "📄";
	if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".png") == 0) return "🖼️";
	if (strcmp(ext, ".mp4") == 0) return "🎥";
	if (strcmp(ext, ".mp3") == 0) return "🎵";
	if (strcmp(ext, ".pdf") == 0) return "📚";
	if (strcmp(ext, ".zip") == 0) return "📦";
	return "📂";
}

// ? This function is used to initialize the packet loss simulator.
void init_packet_loss_simulator(void) {
    srand(time(NULL));
}

// ? This function is used to simulate the loss of a packet.
// ? The function returns true if the packet can be sent, false otherwise.
bool can_send_packet(void) {
    return (rand() % 100 + 1) > LOSS_PROBABILITY;
}

// ? This function is used to get the timeout of the socket in microseconds.
suseconds_t get_timeout(const int32_t sockfd) {
  	struct timeval time;

  	socklen_t size = sizeof(time);
  	time.tv_sec = 0;
  	time.tv_usec = 0;

  	spqr_assert(getsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &time, &size), "getsockopt");

	return time.tv_usec;
}

// ? This function is used to set the timeout of the socket in microseconds.
void set_timeout(const int32_t sockfd, const suseconds_t timeout) {
  	struct timeval time;

  	time.tv_sec = 0;
  	time.tv_usec = timeout;

  	spqr_assert(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time)), "setsockopt");
}

// ? This function is used to set the timeout of the socket in seconds.
void set_seconds_timeout(const int32_t sockfd, const suseconds_t timeout) {
	struct timeval time;

	time.tv_sec = timeout;
	time.tv_usec = 0;

	spqr_assert(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time)), "setsockopt");
}

// ? This function is used to increase the current timeout value in case of adaptive timeout.
void increase_timeout(const int32_t sockfd) {
  	suseconds_t timeout = get_timeout(sockfd);
  	if (timeout <= MAX_TIMEOUT - TIME_UNIT) {
    	set_timeout(sockfd, timeout + TIME_UNIT);
  	}
}

// ? This function is used to decrease the current timeout value in case of adaptive timeout.
void decrease_timeout(const int32_t sockfd) {
  	suseconds_t timeout = get_timeout(sockfd);
  	if (timeout >= MIN_TIMEOUT + TIME_UNIT) {
    	set_timeout(sockfd, timeout - TIME_UNIT);
  	}
}

// ? This function is used to check the return value of a function and exit if it is negative.
void spqr_assert(const int64_t ret, const char *msg) { if (ret < 0) { DIE(msg); } }

// ? This function is used to check the return value of a pointer and exit if it is NULL.
void spqr_assert_ptr(const void *ptr, const char *msg) { if (ptr == NULL) { DIE(msg); } }

// ? This function is used to check the return value of a shm and exit if it is MAP_FAILED.
void spqr_assert_shm(const void *addr, const char *msg) { if (addr == MAP_FAILED) { DIE(msg); } }

// ? This function is used to free a pointer
void spqr_free(char **ptr) { if (*ptr != NULL) { free(*ptr); *ptr = NULL; } }

// ? This function is used to convert a string passed to uppercase.
void spqr_upper(char *msg) {
  	char *ptr = msg;
  	while (*ptr) {
    	*ptr = toupper((uint8_t)*ptr);
    	ptr++;
  	}
}

#ifdef RICK_ROLL
// ? This function is used to execute a command.
void execute_command(const char *command, char *const *argv) {
  	pid_t pid;
  	if (posix_spawn(&pid, command, NULL, NULL, argv, (char *const *)environ) != 0) {
    	DIE("posix_spawn");
  	}

  	int32_t status;
  	spqr_assert(waitpid(pid, &status, 0), "waitpid");

  	if (WIFEXITED(status)) {
    	printf("Command executed with exit code: %d\n", WEXITSTATUS(status));
  	} else {
    	printf("Command did not terminate normally.\n");
  	}
}
#endif
