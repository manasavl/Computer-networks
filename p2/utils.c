#include "message.h"

/* Function to print error messages */
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int getrandom()
{
    int num;
    srand(time(NULL));
    num = (rand() % 5) + 1;
    // printf("Random number: %d\n", num);
    return num;
}

unsigned short crc16(char* data_p, int length)
{
    unsigned char x;
    unsigned short crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ 
               ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc;
}
