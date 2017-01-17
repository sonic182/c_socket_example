#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <unistd.h>

struct client_request {
  int client_fd;
  STAILQ_ENTRY(client_request) tqe;    /* Tail queue. */
} __attribute__ ((__packed__));

STAILQ_HEAD(client_queue, client_request) head;

int main(int argc, char * argv[]){

	struct client_request *n1, *n2, *n3, *np;
	//
	STAILQ_INIT(&head);                     /* Initialize the queue. */
	//
	n1 = malloc(sizeof(struct client_request));      /* Insert at the head. */
	n1->client_fd = 10;
	STAILQ_INSERT_HEAD(&head, n1, tqe);

	n1 = malloc(sizeof(struct client_request));      /* Insert at the tail. */
	n1->client_fd = 20;
	STAILQ_INSERT_TAIL(&head, n1, tqe);
	//
	n2 = malloc(sizeof(struct client_request));      /* Insert after. */
	n2->client_fd = 30;
	STAILQ_INSERT_AFTER(&head, n1, n2, tqe);
	// /* Deletion. */
	STAILQ_REMOVE(&head, n2, client_request, tqe);
	free(n2);
	// /* Deletion from the head. */
	n3 = STAILQ_FIRST(&head);
	STAILQ_REMOVE_HEAD(&head, tqe);
	free(n3);

	STAILQ_FOREACH(np, &head, tqe)
	printf("client fd -> %d\n",np->client_fd);
	/* TailQ Deletion. */
	while (!STAILQ_EMPTY(&head)) {
		n1 = STAILQ_FIRST(&head);
		STAILQ_REMOVE_HEAD(&head, tqe);
		free(n1);
	}
	/* Faster TailQ Deletion. */
	n1 = STAILQ_FIRST(&head);
	while (n1 != NULL) {
		n2 = STAILQ_NEXT(n1, tqe);
		free(n1);
		n1 = n2;
	}
	STAILQ_INIT(&head);

	exit(0);

}
