#include <stdio.h>

/* defines macros like exit() , NULL */
#include <stdlib.h>

/* defines functions like bind */
#include <unistd.h>

/* contains definitions of datatypes used in the sockets */
#include <sys/types.h>

/* contains definitions of structures used in socket */
#include <sys/socket.h>

/* defines the structure hostent used in the program */
#include <netdb.h>

/* constants and structures needed for internet domain addresses */
#include <netinet/in.h>

/* needed for using strlen() functions */
#include <string.h>

#include <netinet/udp.h>
#include <sys/time.h>

typedef struct udp_message_s {
    char ch[10];
    int sequenceNumber;
    int ack;
    unsigned short checksum;
} udp_message_t;

int getrandom();
unsigned short crc16(char* data_p, int length);
void error(const char *msg);
