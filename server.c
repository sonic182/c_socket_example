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
#include <sys/queue.h>

#ifndef NUM_THREADS
#define NUM_THREADS 20
#endif


static volatile int keepRunning = 1; // I don't know what this line does. Somethinb for the compiler I don't care about at this moment.
int sockfd; // main socket file descriptor (for port binding)

pthread_mutex_t queue_lock; // safe access to queue
sem_t semaphore; // keep threads locked if there is no work to do.

// std::queue<int> clisockfds; // queue of client requests (as file descriptors)

// C QUEUE STRUG
struct client_request {
  int client_fd;
  sem_t sem;
  STAILQ_ENTRY(client_request) tqe;    /* Tail queue. */
} __attribute__ ((__packed__));
// QUEUE HEAD DECLARATION
STAILQ_HEAD(client_queue, client_request) head;

// shorcut error call function
void error(const char *msg){
    perror(msg);
    exit(1);
}

// Ctrl + C handler, Doen't close socket with all connections at the time process exits :/
void intHandler(int sig){

  signal(sig, SIG_IGN);
  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);
  exit(0);

}

void *ThreadWork(void* args){
  int n;
  struct client_request *req;
  char buffer[4096];
  char response_buffer[4096];
  int thread_number = *((int*) args);

  while(1){

    sem_wait(&semaphore); // synchronize thread with queue

    // get fd in queue
    pthread_mutex_lock(&queue_lock);
    req = STAILQ_FIRST(&head);
    STAILQ_REMOVE_HEAD(&head, tqe);
    pthread_mutex_unlock(&queue_lock);

    bzero(buffer, 4096);
    bzero(response_buffer, 4096);

    n = read(req->client_fd, buffer, 4095);
    if (n < 0) error("ERROR reading from socket");

    sprintf(response_buffer, "(Thread_%d) Your message was:\n%s", thread_number, buffer);

    n = write(req->client_fd, response_buffer, strlen(response_buffer));
    if (close(req->client_fd) < 0) error("Error closing client fd");

  }
}

int main(int argc, char* argv[]){
  // Signal Ctrl + C to intHandler, exit program.
  signal(SIGINT, intHandler);
  sem_init(&semaphore, 0, 0);
  STAILQ_INIT(&head);

  int portno;
  struct sockaddr_in serv_addr;

  if (argc < 2) {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }

  // open new socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) error("ERROR opening socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  // bind socket to port given
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  // listen gets socket file descriptor and clients queue length for socket, IMPORTANT!
  listen(sockfd, 126);
  printf("Listening on port %d\n", portno);

  // Listen to socket and do the work!
  struct sockaddr_in cli_addr;
  socklen_t clilen;

  // 4096 default memory page size
  char buffer[4096];
  char response_buffer[4096];
  int n, client_fd;

  pthread_t pool[NUM_THREADS];

  for (size_t i = 0; i < NUM_THREADS; i++) {
    /* code */
    pthread_create(&pool[i], NULL, ThreadWork, &i);
  }

  struct client_request *req;

  while (1) {

    clilen = sizeof(cli_addr);

    // Here server keeps waiting
    req = malloc(sizeof(struct client_request));      /* Insert at the head. */
    // req->client_fd = 10;
    req->client_fd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    sem_init(&req->sem, 0, 0);

    if (req->client_fd < 0)
      error("ERROR on accept");


    pthread_mutex_lock(&queue_lock);
    STAILQ_INSERT_HEAD(&head, req, tqe);
    pthread_mutex_unlock(&queue_lock);

    sem_post(&semaphore);

  }

}
