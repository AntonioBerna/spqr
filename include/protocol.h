#pragma once

#include "stdc.h"

typedef enum {
	TRANSFER_ERROR   = -1, // ? Transfer error from sender to receiver or viceversa
	SEQ_NUM_ERROR    = -2, // ? Sequence number error
	WRITE_BYTE_ERROR = -3, // ? Write byte error
} spqr_msg_t;

#define PACKET_SIZE 1500 // ? bytes
#define MAX_RETRIES 10   // ? Maximum number of retries for the last ACK

typedef struct {
	int8_t payload[PACKET_SIZE]; // ? Data payload
	int32_t seq_num;             // ? Sequence number
	int32_t no_packets_to_send;  // ? Total number of packets to send
	int64_t no_bytes_to_send;    // ? Total number of bytes to send
	int64_t size;                // ? Size of the packet
	bool sent;                   // ? Sent flag
	bool received;               // ? Received flag
	bool ack;                    // ? Acknowledged flag
} packet_t;

typedef struct {
	int32_t seq_num;    // ? Sequence number
	int64_t size;       // ? Size of the packet
	int64_t write_byte; // ? Number of bytes written
} ack_packet_t;

// * Receiver Prototypes * //
int8_t receive_data_from_sender(const int32_t sockfd, struct sockaddr_in *receiver_addr, socklen_t receiver_len, packet_t *window, int64_t *size_received, FILE *file);
int8_t main_receiver(const int32_t sockfd, struct sockaddr_in *receiver_addr, const char *pathname);

// * Sender Prototypes * //
void send_data_to_receiver(int32_t sockfd, struct sockaddr_in *sender_addr, packet_t *window, uint32_t window_size);
void wait_ack(int32_t sockfd, struct sockaddr_in *sender_addr, socklen_t len, packet_t *window, uint32_t window_size, FILE *file, int64_t size_file, uint16_t *max_errors, uint64_t *counter, int32_t *no_packets_to_send);
int8_t main_sender(int32_t sockfd, struct sockaddr_in *sender_addr, char *pathname);
