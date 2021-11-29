//
//  client.c
//  ProjectTPS
//
//  Created by Youenn PAUDOIS on 28/11/2021.
//

//#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include<ctype.h>
#define BUFFERSIZE 1000000
#define RCVSIZE 1024
#define WINDOWS_SIZE  4

//Recuperation du numero de port
int str_to_int ( char *let) {
    int i = 0;
    int numb = 0;
 
    if ( let != NULL ) {
        for ( i = 0; (isdigit ( let[i]) == 0) && (let != '\0'); i++);
 
        if ( let[i] != '\0' ) numb = strtol ( &let[i], NULL, 10);
    }
 
    return numb;
}

int main (int argc, char *argv[]) {
    printf("\n");
    printf("==============================================\n");
    printf("=================== CLIENT ===================\n");
    printf("==============================================\n");
    printf("\n");
    
    struct sockaddr_in adresse;
    
    int n;
    int port;
    int port_data = 0;
    port = atoi(argv[1]);
    int valid = 1;
    char buffer[2048];
    char rcv_message[RCVSIZE];
    char snd_message[RCVSIZE];
    char rcv_message_socket_data[RCVSIZE];
    char acquitement[6];
    char *syn_msg = "SYN";
    char *ack_msg = "ACK";
    socklen_t len = sizeof(adresse);
    
    char *file_name = argv[2];

    if (argc < 3) {
        
        printf("EXECUTION : ./client <port> <filename> \n");
        exit(-1);
    }
    
    int socket_control = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_control < 0) {
        perror("ECHEC DE LA CREATION DU SOCKET CONTROLE\n");
        return -1;
    }
    
    setsockopt(socket_control, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

    adresse.sin_family= AF_INET;
    adresse.sin_port= htons(port);
    adresse.sin_addr.s_addr= htonl(INADDR_LOOPBACK);

    sendto(socket_control, (char *) syn_msg, strlen(syn_msg), 0, (struct sockaddr *) &adresse, len);
    printf("CLIENT ENVOI : %s\n", snd_message);
    n = recvfrom(socket_control, (char *) rcv_message, RCVSIZE, 0, (struct sockaddr *) &adresse, &len);
    rcv_message[n] = '\0';
    printf("CLIENT RECOIT : %s\n", rcv_message);

    //Recuperation du numero de port
    
    char *message;
    message = strdup(rcv_message);
    port_data = str_to_int(message);
    printf("%d ok\n",port_data);

    //printf("LE PORT DATA EST %d\n", port_data);
    if(port_data != 0) {
        sendto(socket_control, (char *) ack_msg, strlen(ack_msg), 0, (struct sockaddr *) &adresse, len);
        printf("CLIENT : %s\n", ack_msg);
    }
    
    printf("-------------------------------------\n");
    printf("FIN MESSAGE DE CONTROLE \n");
    printf("-------------------------------------\n");
    /*
        -------------------------------------------------------------
        CREATION DU  SCOKET DE DATA AU NIVEAU DU CLIENT
        -------------------------------------------------------------
    */
    
    int socket_data = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_data < 0) {
        perror("ECHEC DE LA CREATION DU SOCKET DATA\n");
        return -1;
    }
    setsockopt(socket_data, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

    adresse.sin_port= htons(port_data);

    sendto(socket_data, (char *) file_name, strlen(file_name), 0, (struct sockaddr *) &adresse, len);
    n = recvfrom(socket_data, (char *) rcv_message, RCVSIZE, 0, (struct sockaddr *) &adresse, &len);
    rcv_message[n] = '\0';
    int number_of_fragment = atoi(rcv_message);
    int receved_fragment = 0;
    clock_t start, finish;
    start = clock();
    FILE * File = fopen("received.jpeg", "a+");
    while(receved_fragment <= number_of_fragment-1){

        
        n = recvfrom(socket_data, (char *) rcv_message_socket_data, RCVSIZE, 0, (struct sockaddr *) &adresse, &len);
        rcv_message_socket_data[n] = '\0';
    
        printf("RECEIVED: %s FRAGMENT %d\n", &rcv_message_socket_data[6], sizeof(rcv_message_socket_data));
        memcpy(&buffer,&rcv_message_socket_data[6], 20);
        fwrite(&rcv_message_socket_data[6],RCVSIZE,1,File);
    
        printf("\n");
        acquitement[6] = '\0';
        memcpy(&acquitement, &rcv_message_socket_data[0], 6);
        
        sendto(socket_data, (char *) acquitement, 6, 0, (struct sockaddr *) &adresse, len);
        printf("CLIENT ENVOI L'ACK : %s\n", acquitement);
        printf("\n");
        
    
        
        receved_fragment=receved_fragment+1;
               
    }
    
    
    finish = clock();
    double duration;
    duration = finish - start;
    printf("TIME %.3f\n", duration);
    

    double debit;
    debit = (number_of_fragment*1500*8*1000/(1024*duration));
    printf("%.3f kbits par seconde \n", debit);
    close(socket_data);
}
