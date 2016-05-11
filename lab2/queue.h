#ifndef INCLUDED_QUEUE
#define INCLUDED_QUEUE
#include "process.h"

struct node {
	process p;
	struct node * next;
};

typedef struct node node;

typedef struct queue {
	node * head, * tail;
} queue;


queue init();
int empty(queue * q);
process peek(queue * q);
void enqueue(queue * q, process p);
process dequeue(queue * q);
void clear(queue * q);
void find(queue * q, int pid);
process min(queue * q, int min_field);

#endif
