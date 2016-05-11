#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include "queue.h"

queue init()
{
	queue q;
	q.head = NULL;
	q.tail = NULL;
	q.size = 0;

	return q;
}

void clear(queue * q)
{
	node * temp = q->head;
	node * next;
	while (temp) {
		next = temp->next;
		free(temp);
		temp = next;
	}
}

int empty(queue * q)
{
	if (!q->head && !q->tail)
	{
		return 1;
	} else {
		return 0;
	}
}

action * peek(queue * q)
{
	if (!empty(q))
	{
		return &(q->head->p);
	} 
}

//Prepend to tail.
void enqueue(queue * q, action p)
{
	node * temp = (node *) malloc (sizeof(node));
	temp->next = NULL;
	temp->p = p;

	if (q->tail != NULL) 
	{
		q->tail->next = temp;
		q->tail = temp;
	} 
	else
	{
		q->head = q->tail = temp;
	}

	q->size += 1;
}

//Pop from head
action dequeue(queue * q)
{
	if (!empty(q))
	{
		node * temp = q->head;
		action p = q->head->p;
		if (q->head == q->tail)
		{
			q->head = q->tail = NULL;
		} else {
			q->head = q->head->next;
		}
		temp->next = NULL;
		free(temp);
		q->size -= 1;
		return p;
	}

	action empty_queue;
	empty_queue.r_ind = -1;
	return empty_queue;
}

/*
int main(int argc, char **argv)
{
	queue q = init();
	action a1;
	action a2;
	action a3;
	a1.r_ind = 0;
	a2.r_ind = 2;
	a3.r_ind = 1;

	enqueue(&q, a1);
	enqueue(&q, a2);
	enqueue(&q, a3);

	printf("Size after pushes = %d\n", q.size);

	printf("%d\n", dequeue(&q).r_ind);
	printf("%d\n", dequeue(&q).r_ind);
	printf("%d\n", dequeue(&q).r_ind);

	printf("Size after pops = %d\n", q.size);

}
*/







