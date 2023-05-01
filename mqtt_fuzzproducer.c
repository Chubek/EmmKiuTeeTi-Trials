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

typedef char *mqtt_packetbytes_t;
typedef unsigned long mqtt_packetlen_t;
typedef unsigned long mqtt_packetnum_t;
typedef char mqtt_pubackres_t;
typedef unsigned long mqtt_pubackoff_t;

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


typedef struct MQTTFuzzPubackRes {
    mqtt_pubackoff_t writeoffse;
    mqtt_packetnum_t numpubackpackets;
    mqtt_pubackres_t *pubackresults;
} mqtt_pubacks_t;

extern int read_from_file(int fd, void *dst, int readcount, int bytecount);
extern int open_file_path(char *path, int flags);
extern void close_file_desc(int fd);
extern void exit_from_app(int code);
extern void error_out_from_app(int code);
extern int convert_hostaddr_to_network_int(char *host, unsigned int *ipv4);
extern int read_single_packet_from_file(int fd, mqtt_packet_s *packet);
extern void free_single_packet_bytes(mqtt_packet_s *packet);
extern char *memorymap_file_shared(int fd, mqtt_packetnum_t pubacksnum, mqtt_pubackoff_t pubacksoffset);


mqtt_packcoll_s *parse_packet_file(char *filepath) {
    mqtt_packcoll_s packetscoll;
}


int main() {
   int fd = open_file_path("res.bin", O_CREAT);
   mqtt_pubacks_t pubacks;
   pubacks.writeoffset = 10;
   pubacks.numpubackpackets = 3;
   pubacks.pubackresults = calloc(3, 1);
   pubacks.pubackresults[0] = 'F';
   pubacks.pubackresults[1] = 'S';
   pubacks.pubackresults[2] = 'F';
   write_pubacks_to_file(fd, &pubacks);
   free(pubacks.pubackresults);
   close_file_desc(fd);
}