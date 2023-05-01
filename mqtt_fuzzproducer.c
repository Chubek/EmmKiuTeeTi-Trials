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

#define ERROR_READ_SIZE 1
#define ERROR_READ_PACK 2

typedef unsigned short mqtt_packettype_t;
typedef unsigned int mqtt_packetlen_t;
typedef unsigned long mqtt_packetnum_t;
typedef char *mqtt_packetbytes_t;

typedef struct MQTTFuzzPacket {
    mqtt_packetlen_t packet_len;
    mqtt_packetbytes_t packet_bytes;
} mqtt_packet_s;

typedef struct MQTTFuzzPacketColl {
    mqtt_packetnum_t numpublishpackets; 
    mqtt_packet_s *publishpackets;
    mqtt_packet_s connectpacket;
    mqtt_packet_s connackpacket;
    mqtt_packet_s disconnpacket;
    mqtt_packet_s pubackpacket;
} mqtt_packcoll_s;

extern int read_from_file(int fd, void *dst, int readcount, int bytecount);
extern int open_file_path(char *path, int flags);
extern void close_file_desc(int fd);
extern void exit_from_app(int code);
extern void error_out_from_app(int code);
extern int convert_hostaddr_to_network_int(char *host, unsigned int *ipv4);
/*
int read_single_packet(int fd, mqtt_packet_s *packet) {
    int readres = 0;
    readres = read_from_file(fd, &packet->packet_type, 1, 0);
    if (readres != 1) return -1;
    readres = read_from_file(fd, &packet->packet_len, 4, 0);
    if (readres != 4) return -1;
    packet->packet_bytes = calloc(packet->packet_len, 1);
    readres = read_from_file(fd, packet->packet_bytes, packet->packet_len, 0);
    if (readres != packet->packet_len) return -1;
    return 0;
}

mqtt_packetcoll_s parse_mqtt_packet_file(char *filepath) {
    int readres = 0;
    mqtt_packet_s packets;
    
    int fd = open_file_path(filepath);
    if (fd < 1) error_out_from_app(ERROR_OPEN_PACKFILE);
    
    readres = read_from_file(fd, &packets.numpublishpackets, 8, 0);
    if (readres != 8)  error_out_from_app(ERROR_READ_PSIZE);
    mqtt_packet_s *packets = calloc(packets.numpublishpackets, sizeof(mqtt_packet_s));
    for (int i = 0; i < packets.numpublishpackets; i++) {
        readres = read_single_packet(fd, &packets.publishpackets[0]);
        if (readres == -1) error_out_from_app(ERROR_READ_PUBLISHPACK);
    }

    readres = read_single_packet(fd, &packets.connectpacket);
    if (readres == -1) error_out_from_app(ERROR_READ_CONNECTPACK);
    readres = read_single_packet(fd, &packets.connackpacket);
    if (readres == -1) error_out_from_app(ERROR_READ_CONNACKPACK);
    readres = read_single_packet(fd, &packets.disconnpacket);
    if (readres == -1) error_out_from_app(ERROR_READ_DISCONNPACK);
    if (readres == -1) error_out_from_app(ERROR_READ_PUBACKPACK);

    
    close_file_desc(fd);
    return packets;
}   


*/
int main() {
    struct sockaddr_in myaddr;
   inet_aton("125.221.21.255", &myaddr.sin_addr);
    unsigned int ipv4 = 0;
    int res = convert_hostaddr_to_network_int("125.221.21.255", &ipv4);
    printf("%d %u %u\n", res, ipv4, myaddr.sin_addr.s_addr);
}


