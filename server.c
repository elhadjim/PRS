//
//  server.c
//  ProjectTPS
//
//  Created by Youenn PAUDOIS on 28/11/2021.
//

//#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

#define BUFFERSIZE 1000000
#define FRAGMENT_SIZE 1024
#define WINDOWS_SIZE 10
#define RCVSIZE 1024

#define ALPHA_SRTT 0.9
#define RTT 1
#define SRTT_of_zero 4000

int Size_of_file(const char * filename){
    
    FILE * f;
    long int size = 0;
    f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);

    size = ftell(f);

    if (size != -1)
        return(size);
    return 0;
 }

float srtt(int ack_number){

    if (ack_number == 1){

        return ALPHA_SRTT*SRTT_of_zero + (1-ALPHA_SRTT)*RTT;
     
    }
    else{
        return pow(ALPHA_SRTT,ack_number)*(SRTT_of_zero - RTT) + RTT;
        
    }

}


int main (int argc, char *argv[]) {
    printf("\n");
    printf("==============================================\n");
    printf("=================== SERVER ===================\n");
    printf("==============================================\n");
    printf("\n");
    struct sockaddr_in address, client;

    int port;
    port = atoi(argv[1]);
    int valid = 1;

    socklen_t alen = sizeof(client);
    char rcv_message[RCVSIZE];
    char snd_message[RCVSIZE];
    char buffer[BUFFERSIZE];
    char msg[FRAGMENT_SIZE+6];
    char seq_number[6];
    char *syn_ack_msg = "SYN-ACK8080";
    
    if (argc <2) {
        printf("EXECUTION : ./server <port> \n");
        exit(-1);
    }
    /*
        -------------------------------------------------------------
        CREATION DU  SCOKET DE CONTROLE AU NIVEAU DU SERVEUR
        -------------------------------------------------------------
    */
    
    int socket_control = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (socket_control < 0) {
        perror("ECHEC DE LA CREATION DU SOCKET CONTROLE\n");
        return -1;
    }
    setsockopt(socket_control, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

    address.sin_family= AF_INET;
    address.sin_port= htons(port);
    address.sin_addr.s_addr= htonl(INADDR_ANY);

    if (bind(socket_control, (struct sockaddr*) &address, sizeof(address)) == -1) {
        perror("ECHEC DU PROCESSUS DE **BIND**\n");
        close(socket_control);
        return -1;
    }

    int n;
    n = recvfrom(socket_control, rcv_message, RCVSIZE, 0, (struct sockaddr *) &client, &alen);
    rcv_message[n]= '\0';
    printf("SERVER RECOIT : %s\n", rcv_message);
    sendto(socket_control, (const char *)syn_ack_msg, strlen(syn_ack_msg), 0, (const struct sockaddr *) &client, alen);
    printf("SERVER ENVOI: %s\n", syn_ack_msg);
    n = recvfrom(socket_control, rcv_message, RCVSIZE, 0, (struct sockaddr *) &client, &alen);
    rcv_message[n]= '\0';
    printf("SERVER RECOIT : %s\n", rcv_message);

    //check if the client received syn ack msg
    if(strcmp(rcv_message,"ACK") != 0){
        printf("le client a bien recu le port pour communiquer ! \n");
        exit(-1);
    }
    
    printf("-------------------------------------\n");
    printf("FIN MESSAGE DE CONTROLE \n");
    printf("-------------------------------------\n");
    close(socket_control);
    /*
        -------------------------------------------------------------
        CREATION DU  SCOKET DE DATA AU NIVEAU DU SERVEUR
        -------------------------------------------------------------
    */
    int socket_data = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_data < 0) {
        perror("ECHEC DE LA CREATION DU SOCKET DATA\n");
        return -1;
    }
    setsockopt(socket_data, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));
    
    address.sin_port= htons(8080);
        
    if (bind(socket_data, (struct sockaddr*) &address, sizeof(address)) == -1) {
        
        perror("ECHEC DU PROCESSUS DE **BIND**\n");
        close(socket_data);
        return -1;
    }
    // Le serveur recoit le fichier demandé par le client

    n = recvfrom(socket_data, rcv_message, RCVSIZE, 0, (struct sockaddr *) &client, &alen);
    rcv_message[n]= '\0';
    
    /*
        -------------------------------------------------------------
        LECTURE DU FICHIER ET FRAGMENTATION
        -------------------------------------------------------------
    */
    
    //On met le fichier dans un buffer de taille RCVSIZE

    FILE * File = fopen(rcv_message, "r" );

    if ( File == NULL ) {
    
        printf("ECHEC D'OUVERTURE DE FICHIER\n");
        exit(-1);
    }
    
    fread(buffer,BUFFERSIZE,1,File);
    fclose(File);

    //fragmentation et envoi du fichier

    int sent_fragment = 1;
    printf("SIZE OF THE FILE %d\n",Size_of_file(rcv_message));
    int number_of_fragment = Size_of_file(rcv_message)/FRAGMENT_SIZE +1;
    
    sprintf(snd_message, "%d",number_of_fragment);
    sendto(socket_data, (const char *)snd_message, strlen(snd_message), 0, (const struct sockaddr *) &client, alen);
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    int sl;
    while(sent_fragment <= number_of_fragment){

        //for(int i = sent_fragment+1; i<=sent_fragment+WINDOWS_SIZE; i++){
        
        msg[FRAGMENT_SIZE + 6] = '\0';
        sprintf(seq_number, "%06d", sent_fragment);

        memcpy(&msg[0], seq_number, 6);
        memcpy(&msg[6], &buffer[FRAGMENT_SIZE*(sent_fragment-1)], FRAGMENT_SIZE);
        
        sendto(socket_data, (const char *)msg, sizeof(msg), 0, (const struct sockaddr *) &client, alen);
        //sl = select(socket_data, NULL, NULL, NULL, &timeout);
        
        printf("SERVER ENVOIE: %d\n", sizeof(msg));
        
    
        printf("\n");
        n = recvfrom(socket_data, rcv_message, 6, 0, (struct sockaddr *) &client, &alen);
        rcv_message[n] = '\0';
        //if(atoi(rcv_message) == sent_fragment){
        printf("RECEIVED %d SENT\n", atoi(rcv_message));
        printf("\n");
        sent_fragment=sent_fragment+1;
    
    }
    
        
    printf("SERVER RECOIT ACK: %s\n", rcv_message);
    close(socket_data);
    

}




