#include "spqr_server.h"

int32_t main(void) {
    setup_signal_handling(PARENT);
    
    int32_t sockfd = server_create_socket(PORT - 1);
    
	load_ascii_art();
	puts("🌐 Server is listening for incoming connections.");
    
    return server_manage_client_connections(sockfd);
}
