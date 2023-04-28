#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define PUBACK_RETLEN 4
#define CONN_MARKER 1
#define CONNACK_MARKER 2
#define DCONN_MARKER 3
#define PPACK_MARKER 80

typedef int broker_conn_t;
typedef size_t mqtt_packet_size_t;
typedef mqtt_packet_size_t *mqtt_pubpacket_sizes_t;
typedef char* mqtt_packet_t;

typedef struct MQTTFuzzTest {
	size_t num_publish_packets;
	mqtt_publish_packet_size_t pubpackets_sizes_sum;
	mqtt_pubpacket_sizes_t pubpacket_sizes;
	mqtt_packet_t publish_packets;
	mqtt_packet_t puback_packets;
	mqtt_packet_t connect_packet;
	mqtt_packet_t connack_packet;
	mqtt_packet_t disconnect_packet;
} mqtt_fuzzpackets_s;


#define ERROR_OUT(MSG, ...) 									\
	do {														\
		fprintf(sdterr, "\033[1;31mError occurred\033[0m\n");	\
		fprintf(stderr, MSG, __VA_ARGS__);						\
		fprintf(stderr, "\n");									\
		exit(1);												\
	} while(1)

#define CONN_RES_ERROR(CONN_RES) 																				\
	do {																										\
		fprintf(stderr, "\033[1;31mError\033[0 connecting to %s at port %d\n, reason:\033[1m: ", addr, port);	\
		switch (CONN_RES) {																						\
		case EACCESS:																							\
		case EPERM:																								\
			fprintf(stderr, "Access denied");																	\
		case EADDRINUSE:																						\
			fprintf(stderr, "Address in use");																	\
		case EADDRNOTAVAIL:																						\
			fprintf(stderr, "Address not available");															\
		case EAFNOSUPPORT:																						\
			fprintf(stderr, "Protocols don't match");															\
		case EALREADY:																							\
			fprintf(stderr, "A connection has already been made to non-blocking socket");						\
		case EBAD:																								\
			fprintf(stderr, "Socked error");																	\
		case ECONNREFUSED:																						\
			fprintf(stderr, "Connection refused");																\
		case EFAULT:																							\
			fprintf(stderr, "Outside user's address space");													\
		case EINPROGRESS:																						\
			fprintf(stderr, "Another connection is already in progress");										\
		case EINTR:																								\
			fprintf(stderr, "Signal interrupted connection");													\
		case EISCONN:																							\
			fprintf(stderr, "Client socket is already connected");												\
		case ENETUNREACH:																						\
			fprintf(stderr, "Network is unreachable");															\
		case ENOTSOCK:																							\
			fprintf(stderr, "Client socket is not a socket");													\
		case EPROTOTYPE:																						\
			fprintf(stderr, "Client socket does not support protocol");											\
		case ETIMEDOUT:																							\
			fprintf(stderr, "Connection timed out");															\
		default:																								\
			fprintf(stderr, "Unknown error code: %d", CONN_RES);												\
		}																										\
		fprintf(stderr, "\033[0m\n");																			\
		exit(1);																								\
	} while(1)

extern int seek_zero(void *address, size_t size_struct, size_t for_n);
extern size_t sum_array(void *array, size_t size_struct, size_t for_n);

mqtt_fuzzpackets_s new_mqtt_fuzztest(size_t numpackets) {
	return (mqtt_fuzzpackets_s){
		.num_publish_packets = numpackets,
		.pubpackets_sizes_sum = 0, 
		.pubpacket_sizes = calloc(numpackets, sizeof(size_t)),
		.publish_packets = NULL,
		.puback_packets = NULL,
		.connect_packet = NULL,
		.connack_packet = NULL,
		.disconnect_packet = NULL
	};
}

void mqtt_fuzzpackets_parse(mqtt_packet_s *packets_obj, char *filepath) {
	size_t retsize = 0, read_index = 0, curr_packet_size = 0; 
	int has_zero = 0, ddconnsize = 0, 
	char cursor = 0, marker = 0;
	FILE *pfile = fopen(filepath, "r");

	retsize = fread(packets_obj->pubpacket_sizes, sizeof(mqtt_pubpacket_sizes_t), packets_obj->num_publish_packets, pfile);
	if (retsize != packets_obj->num_publish_packets) {
		ERROR_OUT("Wrong number of packets given");
	}

	has_zero = seek_zero((void *)packets_obj->pubpacket_sizes, sizeof(mqtt_publish_packet_size_t), numpackets);
	if (has_zero) {
		ERROR_OUT("Length of a packet may not be zero");
	}

	cursor = fgetc(pfile);
	marker = cursor & 3
	if (marker != CONN_MARKER) {
		ERROR_OUT("CONNECT packet must be specified before CONNACK packet")
	}
	ddconnsize = (cursor & 63) << 4;
	packets_obj->connect_packet = calloc(ddconnsize, sizeof(char));
	fread(packets_obj->connect_packet, sizeof(char), ddconnsize, pfile);

	cursor = fgetc(pfile);
	marker = cursor & 3
	if (marker != CONNACK_MARKER) {
		ERROR_OUT("CONNACK packet must be specified before DISCONNECT packet")
	}
	ddconnsize = (cursor & 63) << 4;
	packets_obj->connack_packet = calloc(ddconnsize, sizeof(char));
	fread(packets_obj->connack_packet, sizeof(char), ddconnsize, pfile);

	cursor = fgetc(pfile);
	marker = cursor & 3
	if (marker != DCONN_MARKER) {
		ERROR_OUT("DISCONNECT packet must be specified PUBLISH packets section")
	}
	ddconnsize = (cursor & 63) << 4;
	packets_obj->disconnect_packet = calloc(ddconnsize, sizeof(char));
	fread(packets_obj->disconnect_packet, sizeof(char), ddconnsize, pfile);

	cursor = fgetc(pfile);
	marker = cursor;
	if (marker != PPACK_MARKER) {
		ERROR_OUT("PUBLISH packets section must start with the specified marker")
	}

	packets_obj->pubpackets_sizes_sum = sum_array((void*)packets_obj->pubpacket_sizes, sizoef(publish_packet_size_t), packets_obj->num_publish_packets);
	packets_obj->publish_packets = calloc(packets_obj->pubpackets_sizes_sum, sizeof(mqtt_publish_packet_size_t));

	
	for (size_t i = 0; i < packets_obj->num_publish_packets; i++) {
		curr_packet_size = packets_obj->pubpacket_sizes[i];
		retsize = fread(&packets_obj->publish_packets[read_index], sizeof(char), curr_packet_size, pfile);
		if (retsize != curr_packet_size) {
			ERROR_OUT("Failed to read packet number %lu of size %lu as denoted by the size headers", i, curr_packet_size);
		}		
		read_index += curr_packet_size;
	}
}

broker_conn_t connect_to_broker(char *addr, int port) {
	struct sockaddr_in conn_addr;
	broker_conn_t conn_socket = 0, int conn_result = 0;

	memset(&conn_addr, 0, sizeof(conn_addr));
	conn_addr.sin_family = AF_INET;
	conn_addr.sin_port = htons(port);
	inet_aton(addr, &conn_addr.sin_addr);

	conn_socket = socket(AF_INET, SOCK_STREAM, 0);
	con_result = connect(conn_socket, &conn_addr, sizeof(conn_addr));

	if (conn_result != 0) {
		CONN_RES_ERROR(conn_result);
	}


	return conn_socket;	
}


int establish_connection_with_broker(mqtt_packet_s *packets_obj)
