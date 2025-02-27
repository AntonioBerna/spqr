#include "spqr_client.h"

int32_t main(int32_t argc, const char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <IPv4>\n", *argv);
		return EXIT_FAILURE;
	}

	setup_signal_handling();

	init_packet_loss_simulator();

	server_response = malloc(MAX_READ_LINE);
	spqr_assert_ptr(server_response, "malloc");
    memset(server_response, 0, MAX_READ_LINE);

	if (!client_establish_connection_with_server(argv[1])) {
		spqr_free(&server_response);
		spqr_assert(close(sockfd), "close");
		return EXIT_FAILURE;
	}

	load_ascii_art();
	printf("🌐 Connection established with the server.\n");
	printf(INFO, WINDOW_SIZE, LOSS_PROBABILITY, TIMEOUT, ADAPTIVE ? "true" : "false");

	return client_manage_server_commands();
}
