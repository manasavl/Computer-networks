#include "message.h"

int main(int argc,char* argv[ ])
{
    int portNumber, sockfd, n, ret, i;
    long fileLength, length_to_send;
    struct sockaddr_in serverAddress, clientAddress;
    struct hostent *server;
    int seqNum = 0;
    int retry = 0;
    socklen_t clilen;
    FILE *fptr;
    udp_message_t msg;
    int ack = -1;
    fd_set rfds;
    struct timeval tv;

    /* Timeout value of 2 seconds */
    /*The timeval structure is used in Windows Sockets by the select function to specify the maximum time the function can take to complete */
    
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    
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
    fseek (fptr, 0, SEEK_END);
    fileLength = ftell (fptr);
    rewind (fptr);
	  
    /* Reading port number */
    portNumber = atoi(argv[4]);

    /* Starting from the address of server address, 
     * till the size mentioned stack memory buffer is made into zero
     */
    bzero((char *)&serverAddress, sizeof(serverAddress));

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
    bcopy((char *) server->h_addr, (char*) &serverAddress.sin_addr.s_addr, 
          server->h_length);

    /* socket file descriptor */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
	   error("Error opening a socket\n");
    }
     
    /* Sending output file name */
    n = sendto(sockfd, argv[2], strlen(argv[2]) + 1, 0, 
               (struct sockaddr*)&serverAddress, sizeof(serverAddress));
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
    n = sendto(sockfd, &length_to_send, sizeof(fileLength), 0, 
               (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (n < 0) {
	   error("Error writing to socket when sending filelength\n");	
    }

    /* Sending file data */
    while(fileLength > 0) {
        if(fileLength > 10 && retry == 0) {
            fread(msg.ch, 1, sizeof(msg.ch), fptr);
            fileLength = fileLength - 10;
        }
        else if (retry == 0) {
            fread(msg.ch, 1, fileLength, fptr);
            fileLength = 0;
        }

        // Set sequence number
        msg.sequenceNumber = seqNum;
        msg.ack = -1;

        // Update checksum
        msg.checksum = crc16(msg.ch, 10);
 //        printf("Got checksum: %d\n", msg.checksum);

        /* Error once in every 5 times */
        n = getrandom();
        if (n == 5) {
            printf("Sending wrong checksum\n");
            msg.checksum = 0;
        }

        printf("sending seqNum: %d\n", seqNum);
        n = sendto(sockfd, &msg, sizeof(udp_message_t), 0,
                       (struct sockaddr *)&serverAddress, 
                       sizeof(serverAddress));
        if (n < 0) {
            printf("Error writing to socket \n");    
        }

	/*Clear all entries from the set.*/
        FD_ZERO(&rfds);
	/* Add fd to the set */
        FD_SET(sockfd, &rfds);
      /*struct timeval at the end  tells select() how long to check the sets.
       *If we have something to  receive before the time interval
       * It returns after the timeout, or when  event occurs, whichever happens first. */
        ret = select(sockfd + 1, &rfds, NULL, NULL, &tv);
//	printf("%d \n", ret);
	if (ret == -1) {
            error("Failed in select");    
        } 
	/*ret=1 and FD_ISSET returns true if fd is in the set */
	else if (ret && FD_ISSET(sockfd, &rfds)) {
            /* Acknowledgement from server */
            n = recvfrom(sockfd, &ack, sizeof(ack), 0,
                 (struct sockaddr *)&clientAddress, &clilen);
            if (n < 0) {
                printf("Error writing to socket \n");    
            }

            /* If ack is not matching data recieved but wrong in server, retry to send message again */
            if (ack != seqNum) {
                printf("retrying the request (Previous ack recieved form server)\n");
                retry = 1;
            } else {
                retry = 0;
                if (seqNum == 0) {
                    seqNum = 1;
                } else {
                    seqNum = 0;
                }
            }
        } else {
	//    printf("%d \n", ret);
            printf("Timeout occured in client\n");
            retry = 1;
        }
    }
    fclose(fptr);
    close(sockfd);

    printf("Completed sending file.....\n\n\n");
    return 0;
}
