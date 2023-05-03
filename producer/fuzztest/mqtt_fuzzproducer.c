#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define O_CREAT 64
#define MAX_UINT16 65535
#define CHECK_YIELD(ERRMSG) if (yield < 0) write_to_stderr_and_exit(ERRMSG)
#define ASSERT_YIELD(VAR, VAL, ERRMSG) if (VAR != VAL) write_to_stderr_and_exit(ERRMSG)
#define CHECK_VAR(VAR, ERRMSG) if (VAR < ) write_to_stderr_and_exit(ERRMSG)
#define CHECK_CMP(ERRMSG) if (!packcmp) write_to_stderr_and_exit(ERRMSG)

typedef void *mqtt_memaddr_t;
typedef void mqtt_nonyield_t;
typedef void mqtt_genobject_t;

typedef signed char *mqtt_message_t;
typedef signed char *mqtt_filepath_t;
typedef signed char *mqtt_hostaddr_t;
typedef signed char mqtt_result_t;
typedef signed char mqtt_yield_t;
typedef signed int mqtt_procid_t;
typedef signed int mqtt_filedsc_t;
typedef signed int mqtt_sockdsc_t;
typedef signed int mqtt_flags_t;
typedef signed int mqtt_exitcode_t;

typedef unsigned char *mqtt_packetbytes_t;
typedef unsigned char *mqtt_results_t;
typedef unsigned char mqtt_packcmp_t;
typedef unsigned short mqtt_netport_t;
typedef unsigned int mqtt_ipv4_t;
typedef unsigned long mqtt_readlen_t;
typedef unsigned long mqtt_writelen_t;
typedef unsigned long mqtt_packetlen_t;
typedef unsigned long mqtt_packetnum_t;
typedef unsigned long mqtt_offset_t;
typedef unsigned long mqtt_addrlen_t;
typedef unsigned long mqtt_memsize_t;

typedef enum MQTTBoolean {
    False = 0,
    True = 1,
} mqtt_boolean_e;

typedef struct MQTTFuzzPacket {
    mqtt_packetlen_t packetlen;
    mqtt_packetbytes_t packetbytes;
} mqtt_packet_s;

typedef mqtt_packet_s *mqtt_publishpacks_t;

typedef struct MQTTFuzzPeripheryPackets {
    mqtt_packet_s connectpacket;
    mqtt_packet_s connackpacket;
    mqtt_packet_s disconnpacket;
    mqtt_packet_s pubackpacket;
} mqtt_peripherypackets_s;


typedef struct MQTTFuzzTestNetConn {
    mqtt_ipv4_t ip;
    mqtt_netport_t port;
    mqtt_sockdsc_t socket;
} mqtt_brokerconn_s;

typedef struct MQTTFuzzPackets {
    mqtt_packetnum_t publishpacketsnum;
    mqtt_publishpacks_t publishpackets;
    mqtt_peripherypackets_s peripherypackets;
} mqtt_fuzzpackets_s;

typedef struct MQTTFuzzContext {
    mqtt_memaddr_t packtsmmap;
    mqtt_results_t resultsmmap;
    mqtt_brokerconn_s brokerconn;
} mqtt_fuzztestctx_s;

extern mqtt_yield_t read_from_file(mqtt_filedsc_t fd, mqtt_memaddr_t dst, mqtt_readlen_t readcount, mqtt_readlen_t bytecount);
extern mqtt_yield_t write_to_file(mqtt_filedsc_t fd, mqtt_memaddr_t dst, mqtt_writelen_t readcount, mqtt_writelen_t bytecount);
extern mqtt_yield_t read_single_packet_from_file(mqtt_filedsc_t fd, mqtt_packet_s *packet);
extern mqtt_yield_t send_packet_to_broker(mqtt_sockdsc_t sock, mqtt_packet_s *packet);
extern mqtt_yield_t receive_packet_from_broker(mqtt_sockdsc_t sock, mqtt_packet_s *packet);
extern mqtt_yield_t convert_hostaddr_to_netbyteorder(mqtt_hostaddr_t host, mqtt_ipv4_t *ipv4conv);
extern mqtt_yield_t write_packet_to_mmaped_file(mqtt_filedsc_t fd, mqtt_packet_s *packet);
extern mqtt_nonyield_t close_file_desc(mqtt_filedsc_t fd);
extern mqtt_nonyield_t exit_from_app(mqtt_exitcode_t code);
extern mqtt_nonyield_t free_single_packet_bytes(mqtt_packet_s *packet);
extern mqtt_nonyield_t zero_out_memory(mqtt_memaddr_t address, mqtt_addrlen_t size);
extern mqtt_nonyield_t write_to_stderr_and_exit(mqtt_message_t message);
extern mqtt_filedsc_t open_file_path(mqtt_filepath_t path, mqtt_flags_t flags);
extern mqtt_memaddr_t memorymap_file_shared(mqtt_filedsc_t fd, mqtt_packetnum_t num, mqtt_offset_t offset);
extern mqtt_memaddr_t allocate_memory(mqtt_memsize_t memsize);
extern mqtt_memaddr_t store_object_in_shared_memory(mqtt_genobject_t object, mqtt_memsize_t num_long_words);
extern mqtt_netport_t netport_to_netbyteorder(mqtt_netport_t port);
extern mqtt_sockdsc_t open_socket_and_connect(mqtt_ipv4_t ipv4addr, mqtt_netport_t port);
extern mqtt_packnum_t read_packet_num(mqtt_filedsc_t fd);
extern mqtt_packcmp_t packet_bytes_are_same(mqtt_packet_s *packet1, mqtt_packet_s *packet2);

mqtt_peripherypackets_s read_periphery_packets(mqtt_filedsc_t fd) {
    mqtt_peripherypackets_s peripherypackets;
    zero_out_memory(&peripherypackets, sizeof(mqtt_peripherypackets_s));
    mqtt_yield_t yield = 0;

    yield = read_single_packet_from_file(fd, &peripherypackets.connectpacket);
    CHECK_YIELD("Error reading CONNECT packet\n");
    yield = read_single_packet_from_file(fd, &peripherypackets.connackpacket);
    CHECK_YIELD("Error reading CONNACK packet\n");
    yield = read_single_packet_from_file(fd, &peripherypackets.disconnpacket);
    CHECK_YIELD("Error reading DISCONNECT packet\n");
    yield = read_single_packet_from_file(fd, &peripherypackets.pubackpacket);
    CHECK_YIELD("Error reading PUBACK packet\n");

    return peripherypackets;
}


mqtt_publishpacks_t read_publish_packets(mqtt_filedsc_t fd, mqtt_packnum_t packetsnum) {
    mqtt_memsize_t allocsize = sizeof(mqtt_packet_s) * packetsnum;
    mqtt_publishpacks_t pubpackets_alloc = allocate_memory(allocsize);
    CHECK_VAR(pubpackets_alloc, "Error allocating memory for packets\n", allocsize);
    zero_out_memory(pubpackets_alloc, allocsize);
    mqtt_yield_t yield = 0;

    for (int i = 0; i < packetsnum; i++) {
        yield = read_single_packet_from_file(fd, &pubpackets_alloc[i]);
        CHECK_YIELD("Error readinch single PUBLISH packet");
    }

    return pubpackets_alloc;
}

mqtt_results_t create_memory_map_for_results(mqtt_filedsc_t fd, mqtt_packnum_t packetsnum, mqtt_offset_t offset) {
    mqtt_memsize_t allocsize = packetsnum * sizeof(mqtt_result_t);
    mqtt_memddr_t mmapped_results = memorymap_file_shared(fd, allocsize, offset);
    CHECK_VAR(mapped_results, "Error allocating file to write results in\n");
    zero_out_memory(mapped_results, allocsize);

    return (mqtt_results_t)mapped_results;
}


mqtt_brokerconn_s create_socket_and_connect(mqtt_ipv4_t ipv4, mqtt_netport_t port) {
    mqtt_sockdsc_t sock = open_socket_and_connect(ipv4, port);
    CHECK_VAR(sock, "Error openning socket and connecting\n");

    return (mqtt_brokerconn_s) {
        .ip = ipv4,
        .port = port,
        .socket = sock,
    };
}

mqtt_fuzzpackets_s read_fuzz_packets(mqtt_filepath_t filepath, mqtt_offset_t offset) {
    mqtt_filedsc_t inpdsc = read_file_path(filepath, 0);
    mqtt_packnum_t publishpacketsnum = read_packet_num(inpdsc);
    mqtt_publishpacks_t publishpackets = read_publish_packets(inpdsc, publishpacketsnum);
    mqtt_peripherypackets_s peripherypackets = read_periphery_packets(inpdsc);
    close_file_desc(inpdsc);

    return (mqtt_fuzzpackets_s){
        .publishpacketsnum = publishpacketsnum,
        .publishpackets = publishpackets,
        .peripherypackets = peripherypackets,
    };
}

mqtt_fuzztestctx_s create_new_fuzz_context(mqtt_memaddr_t packetsmmap, mqtt_results_s resultsmmap, mqtt_ipv4_t ipv4, mqtt_netport_t port) {
    mqtt_brokerconn_t brokerconn = create_socket_and_connect(ipv4, port);
    
    return (mqtt_fuzztestctx_s) {
        .packetsmmap = packetsmmap,
        .resultsmmap = resultsmmap,
        .brokerconn = brokerconn
    };
}

mqtt_nonyield_t establish_connection_with_broker(mqtt_fuzztestctx_s *context, mqtt_boolean_e acknowledge) {
    mqtt_sockdsc_t socket = context->brokerconn.socket;
    mqtt_packet_s *connectpacket = &context->packetsmmap->peripherypackets.connectpacket;
    mqtt_yield_t sendres = send_packet_to_broker(socket, connectpacket);
    CHECK_VAR(sendres, "Error sending a CONNECT packet\n");

    if (!acknowledge) return;

    mqtt_packet_s *connackpacket = &context->packetsmmap->peripherypackets.connackpacket;
    mqtt_packet_s *connack_recv;
    zero_out_memory(connack_recv, sizeof(mqtt_packet_s));
        
    mqtt_yield_t receiveres = receive_packet_from_broker(socket, puback_recv);
    CHECK_VAR(receiveres, "Error receiving a CONNACK packet\n");
    mqtt_packcmp_t packcmp = packet_bytes_are_same(connectpacket, connack_rev);
    CHECK_CMP("Client failed to receive the proper CONNACK\n");
}

mqtt_nonyield_t destablish_connection_with_broker(mqtt_fuzztestctx_s *context) {
    mqtt_sockdsc_t socket = context->brokerconn.socket;
    mqtt_packet_s *disconnpacket = &context->packetsmmap->peripherypackets.disconnpacket;
    mqtt_yield_t sendres = send_packet_to_broker(socket, disconnpacket);
    CHECK_VAR(sendres, "Error sending a DISCONNECT packet\n");
}

mqtt_nonyield_t fuzztest_established_connection(mqtt_fuzztestctx_s *context) {
    mqtt_sockdsc_t socket = context->brokerconn.socket;
    mqtt_packetnum_t publishpacketsnum = context->packetsmmap->publishpacketsnum;
    mqtt_publishpackets_s publishpackets = context->packetsmmap->publishpackets;
    mqtt_packet_s *pubackpacket = context->packetsmmap->pubackpacket;
    mqtt_results_t resultsmmap = context->resultsmmap;
    mqtt_
}

int main() {
    write_to_stderr_and_exit("adsd");
}