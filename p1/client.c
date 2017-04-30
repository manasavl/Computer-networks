#include<stdio.h>

/* defines macros like exit() , NULL */
#include<stdlib.h>

/* defines functions like bind */
#include<unistd.h>

/* contains definitions of datatypes used in the sockets */
#include<sys/types.h>

/* contains definitions of structures used in socket */
#include<sys/socket.h>

/* defines the structure hostent used in the program */
#include<netdb.h>

/* constants and structures needed for internet domain addresses */
#include<netinet/in.h>

/* needed for using strlen() functions */
#include<string.h>

/* Function to print error messages */
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc,char* argv[ ])
{
    int portNumber, sockfd, n, ret;
    long fileLength, length_to_send;
    struct sockaddr_in serverAddress;
    struct hostent *server;
    char message[256];
    char ch[10];
    FILE *fptr;

    if (argc < 5) {
	printf("Usage: ./sftp_client <input filename> <output filename> "
	       "<server address> <server port>\n");
	exit(0);
    }
    
    /* Opening input file */
    fptr = fopen(argv[1], "r");
    if (fptr == NULL) {
	error("Not able to open input file\n");
    }

    /* Finding length of input file */
    fseek (fptr , 0 , SEEK_END);
    fileLength = ftell (fptr);
    rewind (fptr);
	  
    /* Reading port number */
    portNumber = atoi(argv[4]);

    /* Starting from the address of server address, 
     * till the size mentioned stack memory buffer is made into zero
     */
    bzero((char *)&serverAddress, sizeof(serverAddress));
    bzero((char *)ch, 10);
    
    serverAddress.sin_family = AF_INET;
    /* htons is host to network short which converts portnumber in host byte order 
     * to a port number in network byte order
     */
    serverAddress.sin_port = htons(portNumber);
    /* gethostbyname returns a structure of type hostent, 
     * it converts the host given into address
     */
    server = gethostbyname(argv[3]);
    /* 1st argument is source, 2nd argument is destination, 
     * h_length is also a argument in hostent that is returned to server
     */
    bcopy((char *) server->h_addr, (char*) &serverAddress.sin_addr.s_addr, server->h_length);

    /* socket file descriptor */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
	error("Error opening a socket\n");
    }
    
    /* Connecting to server */
    ret = connect(sockfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    if (ret < 0) {
	error("Error connecting to socket\n");
    }

    /* Sending output file name */
    n = write(sockfd, argv[2], strlen(argv[2]) + 1);
    if (n < 0) {
	error("Error writing to socket when sending filename\n");	
    }

    printf("\nInput filename: %s\n", argv[1]);
    printf("Output filename: %s\n", argv[2]);
    printf("Server address: %s\n", argv[3]);
    printf("Server port: %d\n", atoi(argv[4]));
    printf("Filelength: %ld\n", fileLength);

    /* Sending input file length */
    length_to_send = htonl(fileLength);
    n = write(sockfd, &length_to_send, sizeof(fileLength));
    if (n < 0) {
	error("Error writing to socket when sending filelength\n");	
    }

    /* Sending file data */
    while (fileLength > 0) {
	if(fileLength >= 10) {
	    fread(ch, 1, 10, fptr);
	    fileLength = fileLength - 10;
	    n = write(sockfd, ch, sizeof(ch));
	    if (n < 0) {
		printf("Error writing to socket \n");	
	    }
	}
	else {   
	    fread(ch, 1, fileLength, fptr);
	    n = write(sockfd, ch, fileLength);
	    if (n < 0) {
		printf("Error writing to socket \n");	
	    }
	    fileLength = 0;
	}
    }

    fclose(fptr);
    close(sockfd);

    printf("Completed sending file.....\n\n\n");
    return 0;
}
