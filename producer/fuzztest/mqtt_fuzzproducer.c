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
#define PABACK_SUCCESS 'S'
#define PUBACK_FAIL 'F'
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
typedef unsigned int mqtt_procnum_t;
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


typedef struct MQTTFuzzTestProperties {
    mqtt_ipv4_t ip;
    mqtt_netport_t port;
    mqtt_filedsc_t outpdsc;
} mqtt_fuzzprops_s;

typedef struct MQTTFuzzPackets {
    mqtt_packetnum_t publishpacketsnum;
    mqtt_publishpacks_t publishpackets;
    mqtt_peripherypackets_s peripherypackets;
} mqtt_fuzzpackets_s;


typedef struct MQTTFuzzContext {
    mqtt_fuzzpackets_t fuzzpackets;
    mqtt_sockdsc_t socket;
    mqtt_results_t resultsmmap;
} mqtt_fuzztestctx_s;

extern mqtt_yield_t read_from_file(mqtt_filedsc_t fd, mqtt_memaddr_t dst, mqtt_readlen_t readcount, mqtt_readlen_t bytecount);
extern mqtt_yield_t write_to_file(mqtt_filedsc_t fd, mqtt_memaddr_t dst, mqtt_writelen_t readcount, mqtt_writelen_t bytecount);
extern mqtt_yield_t read_single_packet_from_file(mqtt_filedsc_t fd, mqtt_packet_s *packet);
extern mqtt_yield_t send_packet_to_broker(mqtt_sockdsc_t sock, mqtt_packet_s *packet);
extern mqtt_yield_t receive_packet_from_broker(mqtt_sockdsc_t sock, mqtt_packet_s *packet);
extern mqtt_yield_t convert_hostaddr_to_netbyteorder(mqtt_hostaddr_t host, mqtt_ipv4_t *ipv4conv);
extern mqtt_yield_t write_packet_to_mmaped_file(mqtt_filedsc_t fd, mqtt_packet_s *packet);
extern mqtt_nonyield_t fork_process();
extern mqtt_nonyield_t close_file_desc(mqtt_filedsc_t fd);
extern mqtt_nonyield_t close_broker_socket(mqtt_brokerconn_s *brokerconn);
extern mqtt_nonyield_t exit_from_app(mqtt_exitcode_t code);
extern mqtt_nonyield_t free_single_packet_bytes(mqtt_packet_s *packet);
extern mqtt_nonyield_t zero_out_memory(mqtt_memaddr_t address, mqtt_addrlen_t size);
extern mqtt_nonyield_t write_to_stderr_and_exit(mqtt_message_t message);
extern mqtt_nonyield_t unmap_memorymap_shared(mqtt_memaddr_t addr, mqtt_memsize_t memsize);
extern mqtt_filedsc_t open_file_path(mqtt_filepath_t path, mqtt_flags_t flags);
extern mqtt_memaddr_t memorymap_file_shared(mqtt_filedsc_t fd, mqtt_packetnum_t num, mqtt_offset_t offset);
extern mqtt_memaddr_t allocate_memory(mqtt_memsize_t memsize);
extern mqtt_memaddr_t store_object_in_shared_memory(mqtt_genobject_t object, mqtt_memsize_t num_long_words);
extern mqtt_netport_t netport_to_netbyteorder(mqtt_netport_t port);
extern mqtt_sockdsc_t open_socket_and_connect(mqtt_ipv4_t ipv4addr, mqtt_netport_t port);
extern mqtt_packnum_t read_packet_num(mqtt_filedsc_t fd);
extern mqtt_packcmp_t packet_bytes_are_same(mqtt_packet_s *packet1, mqtt_packet_s *packet2);


mqtt_fuzzprops_t new_fuzz_properties(mqtt_hostaddr_t ipaddr, mqtt_netport_t lilendianport, mqtt_filepath_t outfile) {
    mqtt_ipv4_t ip;
    mqtt_yield_t yield = convert_hostaddr_to_netbyteorder(ipaddr, &ip);
    CHECK_YIELD("Error with given IP address\n");

    mqtt_netport_t port = netport_to_netbyteorder(lilendianport);
    if (port >= MAX_UINT16) write_to_stderr_and_exit("Error with given port");

    mqtt_filedsc_t outpdsc = open_file_path(outfile, O_CREAT);
    CHECK_VAR(outpdsc, "Error openning/creating output file");

    return (mqtt_fuzzprops_t) {
        .ip = ip,
        .port = port,
        .outdsc = outdsc,
    }
}

mqtt_nonyield_t close_outpdsc_unmap_resultsptr(mqtt_fuzzprops_s *fuzzprops, mqtt_fuzztestctx_s *context) {
    close_file_desc(fuzzprops->outpdsc);
    unmap_memorymap_shared(context->resultsmmap, sizeof(mqtt_result_t) * context->fuzzpackets.publishpacketsnum);
}

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

mqtt_results_t create_memory_map_for_results(mqtt_filedsc_t outpdsc, mqtt_packnum_t packetsnum, mqtt_offset_t offset) {
    mqtt_memsize_t allocsize = packetsnum * sizeof(mqtt_result_t);
    mqtt_memddr_t mmapped_results = memorymap_file_shared(outpdsc, allocsize, offset);
    CHECK_VAR(mapped_results, "Error allocating file to write results in\n");
    zero_out_memory(mapped_results, allocsize);

    return (mqtt_results_t)mapped_results;
}


mqtt_sockdsc_s create_socket_and_connect(mqtt_ipv4_t ip, mqtt_netport_t port) {
    mqtt_sockdsc_t socket = open_socket_and_connect(ip, port);
    CHECK_VAR(sock, "Error openning socket and connecting\n");

    return socket;
}

mqtt_fuzzpackets_s read_fuzz_packets(mqtt_filepath_t filepath) {
    mqtt_filedsc_t inpdsc = read_file_path(filepath, 0);
    CHECK_VAR(inpdsc, "Error openning input file\n");

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

mqtt_fuzztestctx_s create_new_fuzz_context(mqtt_fuzzpackets_s *fuzzpackets, mqtt_fuzzprops_s *fuzzprops, mqtt_offset_t offset) {
    mqtt_results_t resultsmmap = create_memory_map_for_results(fuzzprops->outdsc, fuzzpackets->publishpacketsnum, offset);
    mqtt_sockdsc_t socket = create_socket_and_connect(fuzzprops->ip, fuzzprops->port);

    return (mqtt_fuzztestctx_s) {
        .fuzzpackets = *fuzzpackets,
        .socket = socket,
        .resultsmmap = resultsmmap,
    };
}

mqtt_nonyield_t establish_connection_with_broker(mqtt_fuzztestctx_s *context, mqtt_boolean_e acknowledge) {
    mqtt_sockdsc_t socket = context->socket;
    mqtt_packet_s *connectpacket = &context->fuzzpackets.peripherypackets.connectpacket;
    mqtt_yield_t sendres = send_packet_to_broker(socket, connectpacket);
    CHECK_VAR(sendres, "Error sending a CONNECT packet\n");

    if (!acknowledge) return;

    mqtt_packet_s *connackpacket = &context->fuzzpackets.peripherypackets.connackpacket;
    mqtt_packet_s *connackrecv;
    zero_out_memory(connackrecv, sizeof(mqtt_packet_s));
        
    mqtt_yield_t receiveres = receive_packet_from_broker(socket, connackrecv);
    CHECK_VAR(receiveres, "Error receiving a CONNACK packet\n");
    mqtt_packcmp_t packcmp = packet_bytes_are_same(connectpacket, connackrev);
    CHECK_CMP("Client failed to receive the proper CONNACK\n");
}

mqtt_nonyield_t destablish_connection_with_broker(mqtt_fuzztestctx_s *context) {
    mqtt_sockdsc_t socket = context->brokerconn.socket;
    mqtt_packet_s *disconnpacket = &context->fuzzpackets.peripherypackets.disconnpacket;
    
    mqtt_yield_t sendres = send_packet_to_broker(socket, disconnpacket);
    CHECK_VAR(sendres, "Error sending a DISCONNECT packet\n");
}
0
mqtt_nonyield_t fuzztest_established_connection(mqtt_fuzztestctx_s *context) {
    mqtt_sockdsc_t socket = context->socket;

    mqtt_packetnum_t publishpacketsnum = context->fuzzpackets.publishpacketsnum;
    mqtt_publishpackets_s publishpackets = context->fuzzpackets.publishpackets;
    mqtt_packet_s *pubackpacket = context->fuzzpackets.pubackpacket;
    
    mqtt_results_t resultsmmap = context->resultsmmap;
    mqtt_packet_s *pubackrecv;
    mqtt_packet_s *currpublishpacket;
    zero_out_mem(pubackrecv, sizeof(mqtt_packet_s));    
    
    mqtt_yield_t sendres = 0, recvres = 0;
    mqtt_packcmp_t packcmp;

    for (int i = 0; i < publishpacketsnum; i++) {
        currpublishpacket = &publishpackets[i];
        sendres = send_packet_to_broker(socket, currpublishpacket);
        if (sendres < 0) {
            resultsmmap[i] = PUBACK_FAIL;
            continue;
        }
        recvres = receive_packet_from_broker(socket, pubackrecv);
        if (recvres < 0) {
            resultsmmap[i] = PUBACK_FAIL;
            continue;
        }
        packcmp = packet_bytes_are_same(currpublishpacket, pubackrecv);
        if (!packcmp) {
            resultsmmap[i] = PUBACK_FAIL;
            continue;
        }
        resultsmmap[i] = PUBACK_SUCCESS;
    }
}

mqtt_nonyield_t fuzztest_roundabout(mqtt_context_s *context, mqtt_bool_t ackconnect) {
    establish_connection_with_broker(context, ackconnect);
    fuzztest_established_connection(context);
    destablish_connection_with_broker(context);
}

mqtt_nonyield_t free_all_packetbytes(mqtt_fuzzpacket_t *fuzzpacket) {
    free_single_packet_bytes(&fuzzpacket->peripherypackets.connectpacket);
    free_single_packet_bytes(&fuzzpacket->peripherypackets.connackpacket);
    free_single_packet_bytes(&fuzzpacket->peripherypackets.disconnpacket);
    free_single_packet_bytes(&fuzzpacket->peripherypackets.pubackpacket);

    for (int i = 0; i < fuzzpacket->publishpacketsnum; i++) {
        free_single_packet_bytes(&fuzzpacket->publishpackets[i]);
    }
}

mqtt_nonyield_t run_fuzztest_across_subprocs(mqtt_filepath_t inputfile, mqtt_filepath_t outfile, mqtt_hostaddr_t host, mqtt_netport_t port, mqtt_procnum_t procnum) {
    mqtt_fuzzpackets_s fuzzpackets = read_fuzz_packets(inputfile);
    mqtt_fuzzprops_s fuzzprops = new_fuzz_properties(host, port, outfile);
    
        


}

int main() {
    write_to_stderr_and_exit("adsd");
}