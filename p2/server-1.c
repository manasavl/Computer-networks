#include "message.h"

int main(int argc, char *argv[])
{
    int sockfd, n, portNumber, i;
    long fileLength = 0;
    int newsockfd;
    char message[256];
    char ch[10];
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    socklen_t clilen;
    FILE *filePtr;
    socklen_t serlen = sizeof(serverAddress);
    udp_message_t msg;
    int ack = 1;
    unsigned short chksum;
    
    if (argc < 2) {
		printf("Usage: ./sftp_server <port no>\n");
		exit(0);
    }

    portNumber = atoi(argv[1]);

    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    /* create socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
		error("Opening Socket");
    }

    /* bind socket to the address */
    n = bind(sockfd, (struct sockaddr *) &serverAddress, 
    	     sizeof(serverAddress));
    if (n < 0) {
		error("Error on binding");
    }

    clilen = sizeof(clientAddress);
    
    while (1) {

		printf("Waiting for client connection...\n");

		/* Reading filename to write */
		bzero(message, 256);
		n = recvfrom(sockfd, message, sizeof(message), 0, 
			         (struct sockaddr *)&clientAddress, &clilen);
		if (n < 0) {
		    error("Error reading from socket");
		}
		
		/* Opening file handle */
		filePtr = fopen(message, "w");
		if (filePtr == NULL) {
		    printf("Not able to open file\n");
		    exit(0);
		}

		/* Receiving file length */
		n = recvfrom(sockfd, &fileLength, sizeof(fileLength), 0, 
			         (struct sockaddr *)&clientAddress, &clilen);		
		if (n < 0) {
			printf("Error reading from socket\n");
		}
		fileLength = ntohl(fileLength);

		/* Converting from network byte order to host byte order */
		printf("Output filename is: %s\n", message);
		printf("Filesize is: %ld\n", fileLength);

		while (fileLength > 0) {
    		n = recvfrom(sockfd, &msg, sizeof(udp_message_t), 0,
    			         (struct sockaddr *)&clientAddress, &clilen);
    		if (n < 0) {
				printf("Error reading from socket\n");
			}

			n = getrandom();

			// Validate checksum
			chksum = crc16(msg.ch, 10);
			if (chksum != msg.checksum) {
				printf("Got wrong checksum\n");
			}
			
			/* Send wrong ack to client */
			if (n == 5 || (chksum != msg.checksum)) {
				printf("Sending wrong ack to client\n");
				n = sendto(sockfd, &ack, sizeof(ack), 0,
	                       (struct sockaddr *)&clientAddress, 
	                       sizeof(clientAddress));
				if (n < 0) {
	            	printf("Error writing to socket\n");    
	        	}
	        	continue;
			}

			/* Skip sending ack message to client */
			if (n == 4) {
				printf("Data not recieved not sending ack to client\n");
				continue;
			}

			/* Update ack on server */
        	if (ack == 0) {
        		ack = 1;
        	} else {
        		ack = 0;
        	}

			// Validate seqNum
			if (msg.sequenceNumber == ack) {
				if (fileLength > 10) {
					fwrite(msg.ch, 1, 5, filePtr);
					fwrite(msg.ch + 5, 1, 5, filePtr);
					fileLength = fileLength - 10;
				} else {
					fwrite(msg.ch, 1, fileLength, filePtr);
					fileLength = 0;
				}
			}
			
			printf("Received seqNum: %d and sent ack\n", ack);
			/* Send acknowledgement to client */
			n = sendto(sockfd, &ack, sizeof(ack), 0,
                       (struct sockaddr *)&clientAddress, 
                       sizeof(clientAddress));
			if (n < 0) {
            	printf("Error writing to socket\n");    
        	}	
		}
		fclose(filePtr);
		printf("Successfully written file...\n\n");
    }

    close(sockfd);
    return 0;
}

