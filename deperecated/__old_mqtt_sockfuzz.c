#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>

extern int seek_zero(void *address, size_t size_struct, size_t for_n);
extern size_t sum_array(void *array, size_t size_struct, size_t for_n);

#define PPUBACK_PACKLEN 4
#define PPUBACK_FAILURE 'F'
#define PPUBACK_SUCCESS 'S' 

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


#define CHECK_FLAG_AGAINST(CURRFLAG, MUSTBE) if (CURRFLAG != MUSTBE) return

typedef enum MQTTStatusFlag {
	CONTX_MQTTFUZZTST_INIT_DONE,
	PARSE_PUBPACKLENS_READ_FAIL,
	PARSE_PUBPACKLENS_ZERO_FAIL,
	PARSE_CONNECTPACK_MARK_FAIL,
	PARSE_CONNECTPACK_READ_FAIL,
	PARSE_CONNACKPACK_MARK_FAIL,
	PARSE_CONNACKPACK_READ_FAIL,
	PARSE_DISCONNPACK_MARK_FAIL,
	PARSE_DISCONNPACK_READ_FAIL,
	PARSE_PUBLISHPACK_MARK_FAIL,
	PARSE_PUBLISHPACK_READ_FAIL,
	BROKR_CONNECTSOCK_OPEN_FAIL,
	BROKR_CONNECTSOCK_CONN_FAIL,
	BROKR_CONNECTPACK_SEND_FAIL,
	BROKR_CONNACKPACK_RECV_FAIL,
	FUZZT_PUBLISHPACK_SEND_FAIL,
	PARSE_ALLMQTTPACK_READ_DONE,
	BROKR_CONNECTSOCK_CONN_DONE,
	BROKR_CONNECTPACK_ESTB_DONE,
	BROKR_CONNECTSOCK_SHUT_DONE,
	BROKR_DISCONNPACK_SEND_DONE,
	FUZZT_PUBLISHPACK_SEND_DONE,
} mqtt_fuzzstatf_e;

typedef size_t mqtt_packetsize_t;
typedef char *mqtt_packet_p;
typedef mqtt_packetsize_t *mqtt_packetsize_p;


typedef char *mqtt_brokerhost_p;
typedef int mqtt_brokersock_t;
typedef int mqtt_brokerport_t;

typedef char *mqtt_fuzzifile_t;
typedef char *mqtt_fuzzofile_t;
typedef uint16_t mqtt_fuzzflags_t;
typedef pid_t mqtt_fuzzprocf_t;

typedef struct MQTTfuzzpacks {
	mqtt_packetsize_t pubpacks_num;
	mqtt_packetsize_t pubpacks_sizes_sum;
	mqtt_packetsize_t connpack_size;
	mqtt_packetsize_t connackpack_size;
	mqtt_packetsize_t dconnpack_size;
	mqtt_packetsize_p pubpacks_sizes;
	mqtt_packet_p puback_results;
	mqtt_packet_p publish_packets;
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
	mqtt_fuzzstatf_e statusf;
	mqtt_fuzzprocf_t procfid;
	mqtt_fuzzflags_t statflg;
	mqtt_fuzzifile_t inpfile;
	mqtt_fuzzofile_t outfile;
	mqtt_fuzzpacks_s packets;
	mqtt_fzzconncx_s connctx;
} mqtt_fuzztstcx_s;


mqtt_fuzzofile_t format_mqtt_outfile(mqtt_fuzzprocf_t procid, char *outdir) {
	char *result;
	asprintf(&result, "%s/mqttfuzz_result_%d.bin", outdir, procid);
	return result;
}

mqtt_fuzzpacks_s new_mqtt_fuzzpacks(mqtt_packsize_t numpackets) {
	return (mqtt_fuzzpacks_s){
		.pubpacks_num = numpackets,
		.pubpacks_sizes = calloc(numpackets, sizeof(size_t)),
		.puback_results = calloc(numpackets, sizeof(char)),
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

mqtt_fuzztstcx_s new_mqtt_fuzztstcx(mqtt_fuzzprocf_t procfid, mqtt_brokersock_t broker_addr, mqtt_brokerport_t broker_port, mqtt_packsize_t numpackets, mqtt_fuzzifile_t inpfile, char *outdir) {
	return (mqtt_fuzztstcx) {
		.statusf = CONTX_MQTTFUZZTST_INIT_DONE,
		.statflg = MAX_UINT16,
		.inpfile = inpfile,
		.outfile = format_mqtt_outdir(procfid, outdir),
		.packets = new_mqtt_fuzzpacks(numpacks),
		.connctx = neq_mqtt_fuzzconnt(broker_addr, broker_port),
	};
}

void mqtt_context_parse_inpfile(mqtt_fuzztstcx_s *context_object) {
	CHECK_FLAG_AGAINST(context_object->statusf, CONTX_MQTTFUZZTST_INIT_DONE);

	size_t retsize = 0, read_index = 0, curr_packet_size = 0; 
	int has_zero = 0;
	char cursor = 0, marker = 0;
	mqtt_fuzzpacks_s *packets_obj = &context_object->packets;

	FILE *pfile = fopen(context_object->inpfile, "r");

	retsize = fread(packets_object->pubpacks_sizes, sizeof(mqtt_packetsize_t), packets_object->pubpacks_num, pfile);
	if (retsize != packets_object->pubpacks_num) {
		context_object->statflg >>= PARSE_FAIL_PACKSIZE;
		goto outer_return;
	}

	has_zero = seek_zero((void *)packets_object->pubpacks_sizes, sizeof(mqtt_packetsize_t), packets_object->pubpacks_num);
	if (has_zero) {
		context_object->statflg >>= PARSE_F;
		goto outer_return;
	}

	void read_connpacks(char cursor, char marker_check, mqtt_packetsize_t *size, mqtt_packet_p *packet, FILE *inpfile, mqtt_fuzzcxstt_e *stflag, int ecode) {
		int marker = cursor & 3
		if (marker != marker_check) {
			*stflag &= ecode;
			goto outer_return;
		}
		*size = (cursor & 63) << 2;
		*packet = calloc(*size, sizeof(char));
		fread(*packet, sizeof(char), *size, inpfile);
	}

	cursor = fgetc(pfile);
	read_connpacks(cursor, CONNECT_MARKER, &packets_object->connpack_size, &packets_object->connect_packet, &context_obj->statflg, PFAILPACKCONN);
	cursor = fgetc(pfile);
	read_connpacks(cursor, CONNACK_MARKER, &packets_object->connack_size, &packets_object->connack_packet, &context_obj->statflg, PFAILPACKCONNACK);
	cursor = fgetc(pfile);
	read_connpacks(cursor, DISCONN_MARKER, &packets_object->dconna_size, &packets_object->disconnect_packet, &context_obj->statflg, PFAILPACKDCONN);
	
	cursor = fgetc(pfile);
	marker = cursor;
	if (marker != PPACK_MARKER) {
		context_obj->statflg = ParseFailPublishFlag;
		goto outer_return;
	}

	packets_object->pubpacks_sizes_sum = sum_array((void*)packets_object->pubpacks_sizes, sizoef(mqtt_packetsize_t), packets_object->pubpacks_num);
	packets_object->publish_packets = calloc(packets_object->pubpacks_sizes_sum, sizeof(mqtt_packetsize_t));

	for (size_t i = 0; i < packets_object->pubpacks_num; i++) {
		curr_packet_size = packets_object->pubpacks_sizes[i];
		retsize = fread(&packets_object->publish_packets[read_index], sizeof(char), curr_packet_size, pfile);
		if (retsize != curr_packet_size) {
			context_object->statflg >>= PFAILPACKPUB;
			goto outer_return;
		}		
		read_index += curr_packet_size;
	}

outer_return:
	fclose(pfile);
	context_object->statflg <<= PSUCCESS;
	return;
}

void mqtt_context_write_outfile(mqtt_fuzztstcx_s *context_object) {
	CHECK_FLAG_AGAINST(context_object->statusf, BROKR_CONNECTSOCK_SHUT_DONE);

	mqtt_fuzzpacket_s *packets_obj = &context_object->packets;
	FILE *ofile = fopen(context_object->outfile, "w");
	size_t result = fwrite(packets_obj->puback_results, sizeof(char), packets_obj->pubpacks_num, ofile);
	if (result != 0) {
		context_object->statflg >>= WFAIL;
		return;
	}
	context_object->statflg <<= WSUCCESS;
}

void mqtt_context_connect_socket(mqtt_fuzztstcx_s *context_object, int quit_on_error) {
	CHECK_FLAG_AGAINST(context_object->statusf, PARSE_ALLMQTTPACK_READ_DONE);

	struct sockaddr_in conn_addr;
	int syscall_ret = 0;
	mqtt_fzzconncx_s *connect_obj = context_object->connctx;

	memset(&conn_addr, 0, sizeof(conn_addr));
	conn_addr.sin_family = AF_INET;
	conn_addr.sin_port = htons(connect_object->port);
	inet_aton(connect_object->port, &conn_addr.sin_addr);

	if ((syscall_ret = socket(AF_INET, SOCK_STREAM, 0) != 0) {
		context_object->statflg >>= CFAILSOCK;
		return;
	}
	connect_object->sock = syscall_ret;

	syscall_ret = 0;
	if ((syscall_ret = connect(conn_socket, &conn_addr, sizeof(conn_addr))) != 0) {
		if (quit_on_error) {
			CONN_RES_ERROR(quit_on_error);
		}
		context_object->statflg >>= CFAILCONN;
		return;
	}

	context_object->statflg <<= CSUCCESS;
}


void mqtt_context_connect_broker(mqtt_fuzztstcx_s *context_object, int retries, int acknowledge) {
	CHECK_FLAG_AGAINST(context_object->statusf, BROKR_CONNECTSOCK_CONN_DONE);

	mqtt_fuzzpackets_s *packets_object = &context_object->packets;
	mqtt_fuzzconntcx_s *connect_object = &context_object->connect;
	size_t return_size  = 0;
	
	do {
		return_size  = send(connect_object->sock, packets_object->connect_packet, packets_object->connpack_size, 0);
	} while (return_size  != packets_object->connack_size || --retries >= 0);

	if (return_size  == packets_object->connect_size) {
		if (acknowledge) {
			char acknowledgement[packets_object->connack_size];
			memset(acknowledgement, 0, packets_object->connack_size);
			
			return_size = 0;
			do {
				return_size = recv(broker_socket, acknowledgement, packets_object->connack_size, 0);
			} while (return_size == packets_object->connack_size);

			if (strncmp(packets_object->connack_packet, acknowledgement, packets_object->connack_size)) {
				goto set_fail;
			}
		} 

		context_object->statflg <<= BSUCCESS;
		return;
	} else if (return_size != packets_object->connack_size) {
		goto set_fail;
	}

set_fail:
	context_object->statflg >>= BFAILACK;
}


void mqtt_context_disconnect_broker(mqtt_fuzztstcx_s *context_object, int retries) {
	CHECK_FLAG_AGAINST(context_object->statusf, FUZZT_PUBLISHPACK_SEND_DONE);

	mqtt_fuzzpackets_s *packets_object = &context_object->packets;
	mqtt_fuzzconntcx_s *connect_object = &context_object->connect;
	size_t return_size = 0;
	
	do {
		return_size  = send(connect_object->sock, packets_object->disconnect_packet, packets_object->dconnpack_size, 0);
	} while (return_size  != packets_object->connack_size || --retries >= 0);
}

void mqtt_context_close_socket(mqtt_fuzztstcx_s *context_object) {
	CHECK_FLAG_AGAINST(context_object->statusf, CONTX_MQTTFUZZTST_INIT_DONE);

	close(context_object->connctx.sock);
	return;
}

void mqtt_context_deallocate_memory(mqtt_fuzztstcx_s *context_object) {
	free(context_object->packets->pubpacks_sizes);
	free(context_object->packets->publish_packets);
	free(context_object->packets->puback_packets);
	free(context_object->packets->connect_packet);
	free(context_object->packets->connack_packet);
	free(context_object->packets->disconnect_packet);
	free(context_object->outfile);
}

void mqtt_context_publish_packets(mqtt_fuzztstcx_s *context_object, int retries, int acknowledge) {
	mqtt_fuzzpackets_s *packets_object = &context_object->packets;
	mqtt_fuzzconntcx_s *connect_object = &context_object->connect;
	size_t return_size  = 0, current_size = 0;
	int read_index = 0, current_retries = 0;

	for (size_t i = 0; i < packets_obj->pubpacks_num; i++) {
		current_size = packets_obj->pubpacks_sizes[i];
		current_retries = retries;

		do {
			return_size = send(connect_object->socket, &packets_obj->publish_packets[read_index], current_size, 0);
		} while (return_size == PPUBACK_PACKLEN || --current_retries >= 0);

		read_index += current_size;

		if (return_size == PPUBACK_PACKLEN) {
			return_size = 0;
			current_retries = retries;

			do {
				return_size = recv(connect_object->socket, acknowledgement, PPUBACK_PACKLEN, 0);
			} while (return_size == PPUBACK_PACKLEN || --current_retries >= 0);


			if (acknowledge) {
				char acknowledgemeent[PPUBACK_PACKLET];
				if (strncmp(acknowledgement, packets_object->puback_packet, PPUBACK_PACKLEN)) {
					goto set_false;
				}
			}

			packets_object->puback_results[i] = PPUBACK_SUCCESS;
		} 			

set_false:
		packets_obj->puback_results[i] = PPUBACK_FAILURE;
	}

	context_object->statflg <<= FSUCCESS;
}


void mqtt_context_fuzztest(mqtt_fuzztstcx_s *context_object, int quit_on_error, int retries, int acknowledge) {
	mqtt_context_parse_inpfile(context_object);
	mqtt_context_connect_socket(context_object, quit_on_error);
	mqtt_context_connect_broker(context_object, retries, acknowledge);
	mqtt_context_publish_packets(context_object, retries, acknowledge);
	mqtt_context_disconnect_broker(context_object, retries);
	mqtt_context_close_socket(context_object);
	mqtt_context_write_outfile(context_object)
	mqtt_context_deallocate_memory(context_object);
	exit(0);
}