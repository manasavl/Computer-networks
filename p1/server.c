#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

int main(int argc, char *argv[])
{
    int sockfd, n, portNumber;
    long fileLength = 0;
    int newsockfd;
    char message[256];
    char ch[5];
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    socklen_t clilen;
    FILE *filePtr;

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
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    /* bind socket to the address */
    n = bind(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    if(n < 0) {
	printf("Error on binding");
    }

    /* listen for connection */
    listen(sockfd, 5);

    while (1) {
	/* accept for connection */
	clilen = sizeof(clientAddress);

	printf("Waiting for client connection...\n");
	newsockfd = accept(sockfd, (struct sockaddr*) &clientAddress, &clilen);
	if (newsockfd < 0) {
	    printf("Error on accept");
	}

	/* Reading filename to write */
	bzero(message, 256);
	n = read(newsockfd, message, sizeof(message));
	if(n < 0) {
	    printf("Error reading from socket");
	}

	/* Opening file handle */
	filePtr = fopen(message, "w");
	if (filePtr == NULL) {
	    printf("Not able to open file\n");
	    exit(0);
	}

	n = read(newsockfd, &fileLength, sizeof(fileLength));
	if (n < 0) {
	    printf("Error reading from socket");
	}
	/* Converting from network byte order to host byte order */
	fileLength = ntohl(fileLength);
	printf("Output filename is: %s\n", message);
	printf("Filesize is: %ld\n", fileLength);

	while (fileLength > 0) {
	    if (fileLength >= 5) {
		n = read(newsockfd, ch, sizeof(ch));
		if (n < 0) {
		    printf("Error reading file content\n");
		}
		fwrite(ch, 1, sizeof(ch), filePtr);
		fileLength = fileLength - sizeof(ch);
	    } else {
		n = read(newsockfd, ch, fileLength);
		if (n < 0) {
		    printf("Error reading file content\n");
		}
		fwrite(ch, 1, fileLength, filePtr);
		fileLength = 0;
	    }
	}
	fclose(filePtr);
	close(newsockfd);
	printf("Successfully written file...\n\n");
    }

    close(sockfd);
    return 0;
}

