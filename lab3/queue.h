#ifndef INCLUDED_QUEUE
#define INCLUDED_QUEUE
#include "action.h"

struct node {
	action p;
	struct node * next;
};

typedef struct node node;

typedef struct queue {
	node * head, * tail;
	int size;
} queue;


queue init();
int empty(queue * q);
action * peek(queue * q);
void enqueue(queue * q, action p);
action dequeue(queue * q);
void clear(queue * q);

#endif
