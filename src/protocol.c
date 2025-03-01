#include "protocol.h"

// ? This function is used to receive the data from the sender and write it to the file.
int8_t receive_data_from_sender(const int32_t sockfd, struct sockaddr_in *receiver_addr, socklen_t receiver_len, packet_t *window, int64_t *size_received, FILE *file) {
	int64_t n;
	bool is_first = true;
	int32_t seq_num;
	int64_t no_bytes_to_send;
	int32_t no_packets_to_receive = 0;
	
#ifdef IS_CLIENT
	printf("\n");
#endif

	uint32_t i = 0;

	do {
		packet_t new_packet = { 0 };
	
	retry:
		n = recvfrom(sockfd, &new_packet, sizeof(new_packet), 0, (struct sockaddr *) receiver_addr, &receiver_len);
		if (n < 0) { HANDLE_ERROR("recvfrom"); }

		if (new_packet.seq_num == SEQ_NUM_ERROR) { return TRANSFER_ERROR; }

		seq_num = new_packet.seq_num;
        if (seq_num >= WINDOW_SIZE) {
            ack_packet_t error_ack = { 0 };
            error_ack.seq_num = SEQ_NUM_ERROR;
            n = sendto(sockfd, &error_ack, sizeof(error_ack), 0, (struct sockaddr *) receiver_addr, receiver_len);
            continue;
        }
        
		no_bytes_to_send = new_packet.no_bytes_to_send;

		// ? If it is the first packet, then set the number of packets to receive.
		if (is_first) {
			no_packets_to_receive = new_packet.no_packets_to_send;
			is_first = !is_first;
		}

		// ? If the packet has been received then send an acknowledgment.
		// ? In particular, if the packet has already been received,
		// ? the size of the packet received is less than the size of the packet to receive,
		// ? or the number of bytes to send is less than or equal to the size received,
		// ? then send an acknowledgment.
		if (window[seq_num].received || (*size_received < window[seq_num].no_bytes_to_send) || (no_bytes_to_send <= *size_received)) {
			ack_packet_t ack = { 0 };
			ack.seq_num = seq_num;
			ack.size = no_bytes_to_send;
			ack.write_byte = *size_received;
			
			// ? Simulate the packet loss.
			if (can_send_packet()) {
				n = sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *) receiver_addr, receiver_len);
				if (n < 0) { HANDLE_ERROR("sendto"); }
			}

			goto retry;
		}

		// ? If the packet has not been received, then store it in the window.
		window[seq_num] = new_packet;
		window[seq_num].received = true;

        // ? Process all available contiguous packets.
        int32_t process_count = 0;
        int32_t max_iterations = WINDOW_SIZE;

		while (window[i].received && process_count++ < max_iterations) {
            packet_t packet = window[i];
            
            *size_received = packet.no_bytes_to_send;
            int64_t num_write = fwrite(&packet.payload, 1, packet.size, file);
            if (num_write != packet.size) {
                HANDLE_ERROR("fwrite");
                return WRITE_BYTE_ERROR;
            }

        #ifdef IS_CLIENT
            static uint32_t packets_processed = 0;
            packets_processed++;
            print_progress(no_packets_to_receive + packets_processed, packets_processed);
        #endif
        
            no_packets_to_receive--;
            window[i].received = false;
            window[i].ack = false;
            i = (i + 1) % WINDOW_SIZE;
        }

		ack_packet_t ack = { 0 };
		ack.seq_num = seq_num;
		ack.size = no_bytes_to_send;
		ack.write_byte = *size_received;
		
		if (can_send_packet()) { // ? Check if the packet is lost.
			n = sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *) receiver_addr, receiver_len);
			if (n < 0) { HANDLE_ERROR("sendto"); }
		    window[seq_num].ack = true;
		}
	
    } while (no_packets_to_receive > 0);

	return EXIT_SUCCESS;
}

// ? This function is used to receive the file from the sender.
int8_t main_receiver(const int32_t sockfd, struct sockaddr_in *receiver_addr, const char *pathname) {
	socklen_t receiver_len = sizeof(*receiver_addr);
	int64_t n;
	int64_t size_received = 0;
	packet_t window[WINDOW_SIZE] = { 0 };

	// ? Open the file in write-binary mode.
	FILE *file = fopen(pathname, "wb");
	if (file == NULL) {
		HANDLE_ERROR("fopen");
		return TRANSFER_ERROR;
	}

	// ? Send the first acknowledgment to the sender.
	n = sendto(sockfd, feedback.START_TRANSFER, strlen(feedback.START_TRANSFER), 0, (struct sockaddr *) receiver_addr, receiver_len);
	if (n < 0) {
        HANDLE_ERROR("sendto");
        fclose(file);
        return TRANSFER_ERROR;
    }
	
	// ? Initialize the window.
	for (int32_t i = 0, seq_num = 0; i < WINDOW_SIZE; ++i) {
		window[i].seq_num = seq_num++;
        window[i].received = false;
        window[i].sent = false;
        window[i].ack = false;
        window[i].no_bytes_to_send = 0;
        window[i].size = 0;
	}

#ifdef IS_CLIENT
	struct timeval end, start;
	spqr_assert(gettimeofday(&start, NULL), "gettimeofday");
#endif

	// ? Receive the data from the sender.
	int8_t ret = receive_data_from_sender(sockfd, receiver_addr, receiver_len, window, &size_received, file);

	spqr_assert(fclose(file), "fclose");

#ifdef IS_CLIENT
	spqr_assert(gettimeofday(&end, NULL), "gettimeofday");
    double time = end.tv_sec - start.tv_sec + (double)(end.tv_usec - start.tv_usec) / 1e6;
    double throughput = (time != 0.0) ? (size_received / time) / SPQR_KB : 0.0; // [kB/s]
#endif

	if (n < 0 || ret == TRANSFER_ERROR) {

	#ifdef IS_CLIENT
		printf("\n\n" FILE_TRANSFER_FAILED, time, throughput);
	#endif

		// ? Delete the file.
		spqr_assert(remove(pathname), "remove");
    
    #ifdef DEBUG
		puts(SPQR_FILENAME_SUCCESSFULLY_DELETED);
	#endif

		return ret;
	}
	
	// ? Send the last acknowledgment to the sender.
    for (int32_t i = 0; i < MAX_RETRIES; ++i) {
        ack_packet_t ack = { 0 };
        ack.seq_num = SEQ_NUM_ERROR;
        ack.write_byte = size_received;
        n = sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *) receiver_addr, receiver_len);
        if (n < 0) { HANDLE_ERROR("sendto"); }
    }

#ifdef IS_CLIENT
    printf("\n\n" FILE_TRANSFER_COMPLETED, time, throughput);
#endif

	return EXIT_SUCCESS;
}

// ? This function is used to send the data to the receiver.
void send_data_to_receiver(int32_t sockfd, struct sockaddr_in *sender_addr, packet_t *window, uint32_t window_size) {
	int64_t n;
	for (uint32_t i = 0; i < window_size; ++i) {
        if (window[i].ack == false) { // ? Check if the packet has been acknowledged.
            if (can_send_packet()) { // ? Check if the packet is lost.
                n = sendto(sockfd, &window[i], sizeof(window[i]), 0, (struct sockaddr *) sender_addr, sizeof(*sender_addr));
                if (n < 0) {
                    HANDLE_ERROR("sendto");
                    continue;
                }
                window[i].sent = true;
                if (window[i].size == 0 || n == 0) { break; } // ? Check if the file has been completely sent.
            }
        }
    }
}

// ? This function is used to wait for the acknowledgment from the receiver.
void wait_ack(int32_t sockfd, struct sockaddr_in *sender_addr, socklen_t len, packet_t *window, uint32_t window_size, FILE *file, int64_t size_file, uint16_t *max_errors, uint64_t *counter, int32_t *no_packets_to_send) {
    int64_t n;
	int32_t ack_num;
    int64_t last_ack_confirmed_bytes = 0;
    uint64_t num_read;
    uint32_t i = 0;
    ack_packet_t new_ack = { 0 };

#ifdef IS_CLIENT
    printf("\n");
#endif

    while (true) {
    wait_for_ack:
        n = recvfrom(sockfd, &new_ack, sizeof(new_ack), 0, (struct sockaddr *) sender_addr, &len); // ? Wait for the acknowledgment from the receiver.
        if (n < 0) { // ? If the acknowledgment is not received.
            (*max_errors)++;
            if (*max_errors >= MAX_ERRORS) { return; }
            if (ADAPTIVE) { increase_timeout(sockfd); }

            // ? Resend the packets that have not been acknowledged.
            for (uint32_t j = 0; j < window_size; ++j) {
                if (window[j].ack == false || window[j].no_bytes_to_send > last_ack_confirmed_bytes) {
                    if (can_send_packet()) { // ? Check if the packet is lost.
                        n = sendto(sockfd, &window[j], sizeof(window[j]), MSG_CONFIRM, (struct sockaddr *) sender_addr, sizeof(*sender_addr));
                        if (n < 0) {
                            HANDLE_ERROR("sendto");
                        } else {
                            window[j].sent = true;
                        }
                    }
                }
            }
            goto wait_for_ack;
        }

        *max_errors = 0;

        // ? Check if the acknowledgment is received.
        if (new_ack.seq_num == SEQ_NUM_ERROR || new_ack.write_byte > size_file || new_ack.write_byte == WRITE_BYTE_ERROR) { return; }
        if (ADAPTIVE) { decrease_timeout(sockfd); }

        ack_num = new_ack.seq_num;
        if (new_ack.size > last_ack_confirmed_bytes) { last_ack_confirmed_bytes = new_ack.size; }

        // ? If the acknowledgment is received, I skip to the next packet.
        if (window[ack_num].ack == true) {
            continue;

        } else {
            window[ack_num].ack = true;
        
        #ifdef IS_CLIENT
            print_progress(size_file, new_ack.write_byte);
        #endif

            while (window[i].ack && *no_packets_to_send > 0) {
                packet_t packet = { 0 };
				
				--(*no_packets_to_send);

				num_read = fread(packet.payload, 1, PACKET_SIZE, file);
                if (num_read == 0) { break; }

                *counter += num_read;
                packet.seq_num = i;
                packet.no_packets_to_send = *no_packets_to_send;
                packet.no_bytes_to_send = *counter;
                packet.size = num_read;
                packet.sent = false;
                packet.ack = false;
                packet.received = false;
                window[i] = packet;

                if (can_send_packet()) { // ? Check if the packet is lost.
                    n = sendto(sockfd, &window[i], sizeof(window[i]), 0, (struct sockaddr *) sender_addr, sizeof(*sender_addr));
                    if (n < 0) {
                        HANDLE_ERROR("sendto");
                    } else {
                        window[i].sent = true;
                    }
                }
                if (window[i].size == 0 || n == 0) { break; } // ? Check if the file has been completely sent.
                
                i = (i + 1) % window_size; // ? Move to the next packet.
            }
        }
    }
}

// ? This function is used to send the data to the receiver.
int8_t main_sender(int32_t sockfd, struct sockaddr_in *sender_addr, char *pathname) {
    socklen_t len = sizeof(*sender_addr);
    int32_t seq_num = 0;
	int64_t n;
    uint16_t max_errors = 0;
    uint64_t counter = 0;
    int32_t no_packets_to_send = 0;

    // ? Set the timeout of the socket.
    set_timeout(sockfd, TIMEOUT);

    // ? Open the file in read-binary mode.
    FILE *file = fopen(pathname, "rb");
    if (file == NULL) {
        HANDLE_ERROR("fopen");
        return TRANSFER_ERROR;
    }

    // ? Calculate the size of the file.
    struct stat st;
    if (fstat(fileno(file), &st) != 0) {
        HANDLE_ERROR("fstat");
        fclose(file);
        return TRANSFER_ERROR;
    }
    int64_t size_file = st.st_size;

    // ? Calculate the number of packets to send.
	no_packets_to_send = (size_file % PACKET_SIZE) ? (size_file / PACKET_SIZE) + 1 : size_file / PACKET_SIZE;

    // ? Create the initial window.
    uint32_t window_size = (no_packets_to_send < WINDOW_SIZE) ? no_packets_to_send : WINDOW_SIZE;
    packet_t window[WINDOW_SIZE] = { 0 };

    // ? Fill the window with the packets.
    for (uint32_t i = 0; i < window_size; ++i) {
        packet_t packet = { 0 };
        
        uint64_t num_read = fread(packet.payload, 1, PACKET_SIZE, file);
        if (num_read == 0) { break; }

        counter += num_read;
        packet.seq_num = seq_num;
        packet.no_packets_to_send = no_packets_to_send;
        packet.no_bytes_to_send = counter;
        packet.size = num_read;
        packet.sent = false;
        packet.ack = false;
        packet.received = false;
        window[i] = packet;

        seq_num = (seq_num + 1) % window_size;
    }

#ifdef IS_CLIENT
    struct timeval end, start;
    spqr_assert(gettimeofday(&start, NULL), "gettimeofday");
#endif

    // ? Start sending the packets.
    send_data_to_receiver(sockfd, sender_addr, window, window_size);
    wait_ack(sockfd, sender_addr, len, window, window_size, file, size_file, &max_errors, &counter, &no_packets_to_send);
    
#ifdef IS_CLIENT
    spqr_assert(gettimeofday(&end, NULL), "gettimeofday");
#endif

    int64_t ret = 0;
    while (true) {
        ack_packet_t new_ack = { 0 };
        n = recvfrom(sockfd, &new_ack, sizeof(new_ack), 0, (struct sockaddr *) sender_addr, &len);
        if (n < 0) { break; }
        ret = new_ack.write_byte;
    }

    spqr_assert(fclose(file), "fclose");
    set_timeout(sockfd, 0);

    if (ret == WRITE_BYTE_ERROR) { return TRANSFER_ERROR; }

#ifdef IS_CLIENT
    double time = end.tv_sec - start.tv_sec + (double)(end.tv_usec - start.tv_usec) / 1e6;
    double throughput = (time != 0.0) ? (size_file / time) / SPQR_KB : 0.0; // [kB/s]
#endif

    if (max_errors >= MAX_ERRORS) {
    
    #ifdef IS_CLIENT
        printf("\n\n" FILE_TRANSFER_FAILED, time, throughput);
    #endif
    
		packet_t packet = { 0 };
        packet.seq_num = SEQ_NUM_ERROR;

        n = sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *) sender_addr, len);
        if (n < 0) { HANDLE_ERROR("sendto"); }
    
        return TRANSFER_ERROR;
    }

#ifdef IS_CLIENT
    printf("\n\n" FILE_TRANSFER_COMPLETED, time, throughput);
#endif

    return EXIT_SUCCESS;
}
