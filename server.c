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

#define RCVSIZE 1024

int main (int argc, char *argv[]) {

  printf("==============================================\n");
  printf("=================== SERVER ===================\n");
  printf("==============================================\n");

  struct sockaddr_in adresse, client;

  int port;

  port = atoi(argv[1]);
  int valid= 1;
  socklen_t alen= sizeof(client);
  char buffer[RCVSIZE];

  if (argc <2) {
    printf("USE: ./server <port> \n");
    exit(-1);
  }
  //create socket
  int server_desc1 = socket(AF_INET, SOCK_STREAM, 0);


  //handle error
  if (server_desc1 < 0) {
    perror("Cannot create socket\n");
    return -1;
  }

  setsockopt(server_desc1, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

  adresse.sin_family= AF_INET;
  adresse.sin_port= htons(port);
  adresse.sin_addr.s_addr= htonl(INADDR_ANY);

  //initialize socket
  if (bind(server_desc1, (struct sockaddr*) &adresse, sizeof(adresse)) == -1) {
    perror("Bind failed\n");
    close(server_desc1);
    return -1;
  }


  //listen to incoming clients
  if (listen(server_desc1,0) < 0) {
    printf("Listen failed\n");
    return -1;
  }

  printf("Listen done\n");
  int child;
  while (1) {

    printf("Accepting\n");
    int client_desc1 = accept(server_desc1, (struct sockaddr*)&client, &alen);

    if (client_desc1 <0){
        printf("ECHEC DE LA CREATION DU SOCKET DATA\n");
        exit(-1);
    }

    child = fork();

    if(child == 0){

        close(server_desc1);
        printf("SOCKET CONTROL %d @IP %s N° Port: %d\n", server_desc1, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        printf("SOCKET DATA %d @IP %s N° Port: %d\n", client_desc1, inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        int msgSize = read(client_desc1,buffer,RCVSIZE);

        while (msgSize > 0) {
          write(client_desc1,buffer,msgSize);
          printf("CLIENT: %s",buffer);
          memset(buffer,0,RCVSIZE);
          msgSize = read(client_desc1,buffer,RCVSIZE);
        }
        exit(-1);
        close(client_desc1);

    }
   }



return 0;
}
