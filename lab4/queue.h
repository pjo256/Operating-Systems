#ifndef INCLUDED_QUEUE
#define INCLUDED_QUEUE
#include "page.h"

struct node {
	page p;
	struct node * next, * previous;
};

typedef struct node node;

typedef struct map {
	struct node ** table;
	int max_size;
} map;

typedef struct queue {
	node * head, * tail;
	map page_to_node;
	int size;
	int max_residents;
} queue;



queue init();
int empty(queue * q);
page * peek(queue * q);
void enqueue(queue * q, page p);
page dequeue(queue * q);
void clear(queue * q);
int reference_page(queue * q, page p);
#endif
