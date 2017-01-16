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
#include <pthread.h>
#include <semaphore.h>
#include <queue>          // std::queue

#ifndef NUM_THREADS
#define NUM_THREADS 4
#endif

static volatile int keepRunning = 1;
int sockfd;

pthread_mutex_t queue_lock;
sem_t semaphore;

// std::vector<int> clisockfds;
std::queue<int> clisockfds;


void error(const char *msg){
    perror(msg);
    exit(1);
}

void intHandler(int sig){

  signal(sig, SIG_IGN);
  close(sockfd);
  exit(0);

}

void *ThreadWork(void* args){
  int client_fd, n;
  char buffer[256];
  char response_buffer[256];
  int thread_number = *((int*) args);
  while(1){
    // work if items in queue.
    sem_wait(&semaphore);

    // get fd in queue
    pthread_mutex_lock(&queue_lock);
    client_fd = clisockfds.front();
    clisockfds.pop();
    pthread_mutex_unlock(&queue_lock);

    bzero(buffer, 256);
    bzero(response_buffer, 256);

    n = read(client_fd, buffer, 255);
    if (n < 0) error("ERROR reading from socket");

    sprintf(response_buffer, "(Thread_%d) Your message was:\n%s", thread_number, buffer);

    n = write(client_fd, response_buffer, strlen(response_buffer));
    close(client_fd);

  }
}

int main(int argc, char* argv[]){
  // Signal Ctrl + C to intHandler, exit program.
  signal(SIGINT, intHandler);
  sem_init(&semaphore, 0, 0);

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

  pthread_t pool[NUM_THREADS];

  for (size_t i = 0; i < NUM_THREADS; i++) {
    /* code */
    pthread_create(&pool[i], NULL, ThreadWork, &i);
  }

  // for (size_t i = 0; i < NUM_THREADS; i++) {
  //   /* code */
  //   pthread_join(pool[i], NULL);
  // }
  // printf("dead threads\n");

  while (1) {

    clilen = sizeof(cli_addr);

    // Here server keeps waiting
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0)
      error("ERROR on accept");

    pthread_mutex_lock(&queue_lock);
    clisockfds.push(newsockfd);
    pthread_mutex_unlock(&queue_lock);

    sem_post(&semaphore);

  }

}
