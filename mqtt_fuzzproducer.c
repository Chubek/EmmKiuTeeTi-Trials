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

typedef void *mqtt_voidptr_t;
typedef void *mqtt_memaddr_t;
typedef void mqtt_nonyield_t;

typedef signed char *mqtt_filepath_t;
typedef signed char *mqtt_hostaddr_t;
typedef signed char mqtt_yield_t;
typedef signed int mqtt_procid_t;
typedef signed int mqtt_filedsc_t;
typedef signed int mqtt_sockdsc_t;
typedef signed int mqtt_flags_t;
typedef signed int mqtt_exitcode_t;

typedef unsigned char *mqtt_packetbytes_t;
typedef unsigned char mqtt_pubackres_t;
typedef unsigned short mqtt_netport_t;
typedef unsigned int mqtt_ipv4_t;
typedef unsigned int mqtt_readlen_t;
typedef unsigned int mqtt_writelen_t;
typedef unsigned long mqtt_packetlen_t;
typedef unsigned long mqtt_packetnum_t;
typedef unsigned long mqtt_pubackoff_t;

typedef struct MQTTFuzzPacket {
    mqtt_packetlen_t packet_len;
    mqtt_packetbytes_t packet_bytes;
} mqtt_packet_s;

typedef struct MQTTFuzzPeripheryPackets {
    mqtt_packet_s connectpacket;
    mqtt_packet_s connackpacket;
    mqtt_packet_s disconnpacket;
    mqtt_packet_s pubackpacket;
} mqtt_peripherypackets_s;

typedef struct MQTTFuzzPublishPackets {
    mqtt_packetnum_t publishpacketsnum; 
    mqtt_packet_s *publishpackets;
} mqtt_publishpackets_s;

typedef struct MQTTFuzzTest {
    mqtt_peripherypacket_s peripherypackets;
    mqtt_publishpackets_s publishpackets;
} mqtt_fuzztest_s;

extern mqtt_yield_t read_from_file(mqtt_filedsc_t fd, mqtt_voidptr_t dst, mqtt_readlen_t readcount, mqtt_readlen_t bytecount);
extern mqtt_yield_t open_file_path(mqtt_filepath_t path, mqtt_flags_t flags);
extern mqtt_nonyield_t close_file_desc(mqtt_filedsc_t fd);
extern mqtt_nonyield_t exit_from_app(mqtt_exitcode_t code);
extern mqtt_yield_t read_single_packet_from_file(mqtt_filedsc_t fd, mqtt_packet_s *packet);
extern mqtt_nonyield_t free_single_packet_bytes(mqtt_packet_s *packet);
extern mqtt_memaddr_t memorymap_file_shared(mqtt_filedsc_t fd, mqtt_packetnum_t pubacksnum, mqtt_pubackoff_t pubacksoffset);
extern mqtt_yield_t send_packet_to_broker(mqtt_sockdsc_t sock, mqtt_packet_s *packet);
extern mqtt_yield_t receive_packet_from_broker(mqtt_sockdsc_t sock, mqtt_packet_s *packet);
extern mqtt_yield_t convert_hostaddr_to_netbyteorder(mqtt_hostaddr_t host, mqtt_ipv4_t *ipv4conv);
extern mqtt_netport_t netport_to_netbyteorder(mqtt_netport_t port);
extern mqtt_sockdsc_t open_socket_and_connect(mqtt_ipv4_t ipv4addr, mqtt_netport_t port);
extern mqtt_yield_t write_packet_to_mmaped_file(mqtt_filedsc_t fd, mqtt_packet_s *packet);


int main() {
   unsigned short port = 42111;
   unsigned short netport = htons(port);
   mqtt_netport_t cstport = netport_to_netbyteorder(port);

   printf("%hu %hu\n", netport, cstport);
}