/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

static volatile int keepRunning = 1;
int sockfd;

void error(const char *msg){
    perror(msg);
    exit(1);
}

void intHandler(int sig){

  signal(sig, SIG_IGN);
  close(sockfd);
  exit(0);

}

int main(int argc, char* argv[]){
  // Signal Ctrl + C to intHandler, exit program.
  signal(SIGINT, intHandler);

  // GET SOCKET
  int portno = 3000;
  struct sockaddr_in serv_addr;

  if (argc < 2) {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }

  // open new socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  // bind socket to port given
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  // listen gets file descriptor and queue length, IMPORTANT!
  listen(sockfd, 126);
  printf("Listening on port %d\n", portno);

  // Listen to socket and do the work!
  struct sockaddr_in cli_addr;
  socklen_t clilen;

  // 4096 default memory page size
  char buffer[4096];
  char response_buffer[4096];
  int n, newsockfd;

  while (1) {

    clilen = sizeof(cli_addr);

    // Here server keeps waiting
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0)
      error("ERROR on accept");

    bzero(buffer, 4096);
    bzero(response_buffer, 4096);
    n = read(newsockfd, buffer, 4095);

    if (n < 0) error("ERROR reading from socket");
      sprintf(response_buffer, "Your message was:\n%s", buffer);

    n = write(newsockfd, response_buffer, strlen(response_buffer));
    close(newsockfd);
  }

}
