#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <assert.h>

#include "common.h"
#include "packet.h"
int window_size = 100000; // rcvr window size

/*
 * You ar required to change the implementation to support
 * window size greater than one.
 * In the currenlt implemenetation window size is one, hence we have
 * onlyt one send and receive packet
 */
tcp_packet *recvpkt;
tcp_packet *sndpkt;

int main(int argc, char **argv) {
    int sockfd; /* socket */
    int portno; /* port to listen on */
    int clientlen; /* byte size of client's address */
    struct sockaddr_in serveraddr; /* server's addr */
    struct sockaddr_in clientaddr; /* client addr */
    int optval; /* flag value for setsockopt */
    FILE *fp; // pointer to a file to write data in
    char buffer[MSS_SIZE];
    struct timeval tp;
   
    /*
     * check command line arguments
     */
    if (argc != 3) {
        fprintf(stderr, "usage: %s <port> FILE_RECVD\n", argv[0]);
        exit(1);
    }
    portno = atoi(argv[1]);

    fp  = fopen(argv[2], "w");
    if (fp == NULL) {
        error(argv[2]);
    }

    /*
     * socket: create the parent socket
     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* setsockopt: Handy debugging trick that lets
     * us rerun the server immediately after we kill it;
     * otherwise we have to wait about 20 secs.
     * Eliminates "ERROR on binding: Address already in use" error.
     */
    optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
            (const void *)&optval , sizeof(int));

    /*
     * build the server's Internet address
     */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

    /*
     * bind: associate the parent socket with a port
     */
    if (bind(sockfd, (struct sockaddr *) &serveraddr,
                sizeof(serveraddr)) < 0)
        error("ERROR on binding");

    /*
     * main loop: wait for a datagram, then echo it
     */
    VLOG(DEBUG, "epoch time,  data size,  seqno  \n");
    int* window = (int*)malloc(sizeof(int)*window_size); // will be used as a circular buffer
    
    for (int i = 0; i < window_size; i++){
        window[i] = 0; //init
    }
    int last_ack = 0;// last ACKed packet number to send to sender
    int index = 0;
    clientlen = sizeof(clientaddr);
    int window_start = 0;//start of the queue
    while (1) {
        /*
         * recvfrom: receive a UDP datagram from a client
         */
        //VLOG(DEBUG, "waiting from server \n");
        if (recvfrom(sockfd, buffer, MSS_SIZE, 0,
                (struct sockaddr *) &clientaddr, (socklen_t *)&clientlen) < 0) {
            error("EROR in recvfrom");
        }
        recvpkt = (tcp_packet *) buffer;
        assert(get_data_size(recvpkt) <= DATA_SIZE);
        
       
        
        /*
         * sendto: ACK back to the client
         */
        gettimeofday(&tp, NULL);
        VLOG(DEBUG, "%lu, %d, %d", tp.tv_sec, recvpkt->hdr.data_size, recvpkt->hdr.seqno);
        
        if ( recvpkt->hdr.data_size == 0) { //check if it is the last packet
                   VLOG(INFO, "Successfully received the file!");
                   fclose(fp);
                   break;
               }
        
        int ackno = 0;
        if(recvpkt->hdr.seqno != 0){
            ackno =(recvpkt->hdr.seqno) / DATA_SIZE;
           }
        else{//the very first packet recvd
            ackno = 0;
        }
        int interval = 0;
        if (last_ack<= ackno){ //if new packet
               
            if(window[(window_start + (ackno - last_ack))% window_size] == 0){
                fseek(fp, recvpkt->hdr.seqno, SEEK_SET);
                fwrite(recvpkt->data, 1, recvpkt->hdr.data_size, fp);//writes packet data into file
                window[(window_start + (ackno - last_ack))% window_size] = 1;// mark packet received
            }
            
                
            while (window[window_start] == 1){
                interval ++;
                window[window_start] = 0; // got ACKed and now renew
                window_start = (window_start + 1) % window_size;//goes to the next
            }
            last_ack = last_ack + interval;
        }
        sndpkt = make_packet(0);
        sndpkt->hdr.ctr_flags = ACK;
        sndpkt->hdr.ackno = last_ack;
        
        if (sendto(sockfd, sndpkt, TCP_HDR_SIZE, 0,
                (struct sockaddr *) &clientaddr, clientlen) < 0) {
            error("ERROR in sendto");
        }
        
        
        
        }

    return 0;
}
