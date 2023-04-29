#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern int seek_zero(void *address, size_t size_struct, size_t for_n);
extern size_t sum_array(void *array, size_t size_struct, size_t for_n);

#define PPUBACK_PACKLEN 4

#define CONNECT_MARKER 1
#define CONNACK_MARKER 2
#define DISCONN_MARKER 3
#define PUBPACK_MARKER 80

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


typedef size_t mqtt_packetsize_t;
typedef char *mqtt_packet_p;
typedef mqtt_packetsize_t *mqtt_packetsize_p;


typedef char *mqtt_brokerhost_p;
typedef int mqtt_brokersock_t;
typedef int mqtt_brokerport_t;

typedef char *mqtt_fuzzifile_t;
typedef char *mqtt_fuzzofile_t;

typedef enum MQTTFuzzContextStatus {
	CtxInitialized = 0,
	ParseFailPackSize = 1,
	ParseFailPakcZero = 6,
	ParseFailConnectPack = 2,
	ParseFailConnackpack = 3,
	ParseFailDisconnectPack = 4,
	ParseFailPublishPack = 5,
	ParseFailPublishFlag = 7;
} mqtt_fuzzcxstt_e;


typedef struct MQTTfuzzpacks {
	mqtt_packetsize_t pubpacks_num;
	mqtt_packetsize_t pubpacks_sizes_sum;
	mqtt_packetsize_t connpack_size;
	mqtt_packetsize_t connackpack_size;
	mqtt_packetsize_t dconnpack_size;
	mqtt_packetsize_p pubpacks_sizes;
	mqtt_packet_p publish_packets;
	mqtt_packet_p puback_packets;
	mqtt_packet_p connect_packet;
	mqtt_packet_p connack_packet;
	mqtt_packet_p disconnect_packet;
} mqtt_fuzzpacks_s;

typedef struct MQTTFuzzConn {
	mqtt_brokersock_t sock,
	mqtt_brokerhost_t host;
	mqtt_brokerport_t port,
} mqtt_fuzzconnt_s;

typedef struct MQTTFuzzTestContext {
	mqtt_fuzzcxstt_e statusn;
	mqtt_fuzzifile_t inpfile;
	mqtt_fuzzofile_t outfile;
	mqtt_fuzzpacks_s packets;
	mqtt_fzzconncx_s connctx;
} mqtt_fuzztstcx_s;






mqtt_fuzzpacks_s new_mqtt_fuzzpacks(mqtt_packsize_t numpackets) {
	return (mqtt_fuzzpacks_s){
		.pubpacks_num = numpackets,
		.pubpacks_sizes = calloc(numpackets, sizeof(size_t)),
		.puback_packets = calloc(numpackets * PPUBACK_PACKLEN, sizeof(char)),
		.pubpacks_sizes_sum = 0, 
		.connpack_size = 0,
		.connack_size = 0,
		.dconnpack_size = 0,
		.publish_packets = NULL,
		.connect_packet = NULL,
		.connack_packet = NULL,
		.disconnect_packet = NULL
	};
}

mqtt_fuzzconnt_s new_mqtt_fuzzconnt(mqtt_brokersock_t broker_addr, mqtt_brokerport_t broker_port) {
	return (mqtt_fuzzconnt_s) {
		.host = broker_addr,
		.port = broker_port,
		.sock = 0
	};
}

mqtt_fuzztstcx_s new_mqtt_fuzztstcx(mqtt_brokersock_t broker_addr, mqtt_brokerport_t broker_port, mqtt_packsize_t numpackets, mqtt_fuzzifile_t inpfile, mqtt_fuzzofile_t outfile) {
	return (mqtt_fuzztstcx) {
		.statusn = CtxInitialized,
		.inpfile = inpfile,
		.outfile = outfile,
		.packets = new_mqtt_fuzzpacks(numpacks),
		.connctx = neq_mqtt_fuzzconnt(broker_addr, broker_port),
	};
}

void mqtt_context_parse_inpfile(mqtt_fuzztstcx_s *context_object) {
	size_t retsize = 0, read_index = 0, curr_packet_size = 0; 
	int has_zero = 0;
	char cursor = 0, marker = 0;
	mqtt_fuzzpacks_s *packets_obj = &context_object->packets;

	FILE *pfile = fopen(context_object->inpfile, "r");

	retsize = fread(packets_object->pubpacks_sizes, sizeof(mqtt_packetsize_t), packets_object->pubpacks_num, pfile);
	if (retsize != packets_object->pubpacks_num) {
		context_object->statusn = PFAILPACKSIZE;
		goto outer_return;
	}

	has_zero = seek_zero((void *)packets_object->pubpacks_sizes, sizeof(mqtt_packetsize_t), packets_object->pubpacks_num);
	if (has_zero) {
		context_object->statusn = PFAILPACKZERO;
		goto outer_return;
	}

	void read_connpacks(char cursor, char marker_check, mqtt_packetsize_t *size, mqtt_packet_p *packet, FILE *inpfile, mqtt_fuzzcxstt_e *stflag, int ecode) {
		int marker = cursor & 3
		if (marker != marker_check) {
			*stflag = ecode;
			goto outer_return;
		}
		*size = (cursor & 63) << 4;
		*packet = calloc(*size, sizeof(char));
		fread(*packet, sizeof(char), *size, inpfile);
	}

	cursor = fgetc(pfile);
	read_connpacks(cursor, CONNECT_MARKER, &packets_object->connpack_size, &packets_object->connect_packet, &context_obj->statusn, PFAILPACKCONN);
	cursor = fgetc(pfile);
	read_connpacks(cursor, CONNACK_MARKER, &packets_object->connack_size, &packets_object->connack_packet, &context_obj->statusn, PFAILPACKCONNACK);
	cursor = fgetc(pfile);
	read_connpacks(cursor, DISCONN_MARKER, &packets_object->dconna_size, &packets_object->disconnect_packet, &context_obj->statusn, PFAILPACKDCONN);
	
	cursor = fgetc(pfile);
	marker = cursor;
	if (marker != PPACK_MARKER) {
		context_obj->statusn = ParseFailPublishFlag;
		goto outer_return;
	}

	packets_object->pubpacks_sizes_sum = sum_array((void*)packets_object->pubpacks_sizes, sizoef(mqtt_packetsize_t), packets_object->pubpacks_num);
	packets_object->publish_packets = calloc(packets_object->pubpacks_sizes_sum, sizeof(mqtt_packetsize_t));

	for (size_t i = 0; i < packets_object->pubpacks_num; i++) {
		curr_packet_size = packets_object->pubpacks_sizes[i];
		retsize = fread(&packets_object->publish_packets[read_index], sizeof(char), curr_packet_size, pfile);
		if (retsize != curr_packet_size) {
			context_object->statusn = PFAILPACKPUB;
			goto outer_return;
		}		
		read_index += curr_packet_size;
	}

context_object->statusn = PSUCCESS;

outer_return:
	fclose(pfile);
	return;
}

void mqtt_context_connect_socket(mqtt_fuzztstcx_s *context_object, int quit_on_error) {
	struct sockaddr_in conn_addr;
	int syscall_ret = 0;
	mqtt_fzzconncx_s *connect_obj = context_object->connctx;

	memset(&conn_addr, 0, sizeof(conn_addr));
	conn_addr.sin_family = AF_INET;
	conn_addr.sin_port = htons(connect_object->port);
	inet_aton(connect_object->port, &conn_addr.sin_addr);

	if ((syscall_ret = socket(AF_INET, SOCK_STREAM, 0) != 0) {
		context_object->statusn = CFAILSOCK;
		goto outer_ret;
	}
	connect_object->sock = syscall_ret;

	syscall_ret = 0;
	if ((syscall_ret = connect(conn_socket, &conn_addr, sizeof(conn_addr))) != 0) {
		if (quit_on_error) {
			CONN_RES_ERROR(quit_on_error);
		}
		context_object->statusn = CFAILCONN;
		goto outer_ret;
	}

	context_object->statusn = CSUCCESS;

outer_ret:
	return;	
}


void mqtt_context_connect_broker(mqtt_fuzztstcx_s *context_object, int retries, int acknowledge) {
	mqtt_fuzzpackets_s *packets_object = &context_object->packets;
	size_t return_size  = 0;
	
	do {
		return_size  = send(broker_socket, packets_object->connect_packet, packets_object->connpack_size, 0);
	} while (return_size  != packets_object->connack_size || --retries >= 0);

	if (return_size  == packets_object->connack_size && acknowledge) {
		char acknowledgement[packets_object->connack_size];
		memset(acknowledgement, 0, packets_object->connack_size);
		
		return_size = 0;
		do {
			return_size = recv(broker_socket, acknowledgement, packets_object->connack_size, 0);
		} while (return_size == packets_object->connack_size);

		if (strncmp(packets_object->connack_packet, acknowledgement, packets_object->connack_size)) {
			context_object->statusn = BFAILACK;
			goto outer_ret;
		}
	} else {
		context_object->statusn = BESTABLISH;
	}

	context_obj->statusn = BSUCCESS;

outer_ret:
	return;
}


void mqtt_context_disconnect_broker(mqtt_fuzztstcx_s *context_object, int retries) {
	mqtt_fuzzpackets_s *packets_object = &context_object->packets;
	size_t return_size  = 0;
	
	do {
		return_size  = send(broker_socket, packets_object->disconnect_packet, packets_object->dconnpack_size, 0);
	} while (return_size  != packets_object->connack_size || --retries >= 0);

	if (return_size == packets_object->dconnpack_size) {
		context_obj->statusn = DCONNFAIL;
		return;
	}

	packets_object->statusn = DCONNSUCCESS;
	return;
}

void mqtt_context_close_socket(mqtt_fuzztstcx_s *context_object) {
	close(context_object->connctx.sock);
	context_object->statusn = SCLOSED;
	return;
}

void mqtt_context_deallocate_memory(mqtt_fuzztstcx_s *context_object) {
	free(context_object->packets->pubpacks_sizes);
	free(context_object->packets->publish_packets);
	free(context_object->packets->puback_packets);
	free(context_object->packets->connect_packet);
	free(context_object->packets->connack_packet);
	free(context_object->packets->disconnect_packet);
	return 0;
}

