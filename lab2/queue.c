#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include "queue.h"

queue init()
{
	queue q;
	q.head = NULL;
	q.tail = NULL;

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

void find(queue * q, int pid)
{
	node * temp = q->head;
	node * next;
	while (temp) {
		next = temp->next;
		if (temp->p.pid == pid) 
		{
			printf("Found %d\n", pid);
			return;
		}
		temp = next;
	}
	printf("Not found\n");
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

process peek(queue * q)
{
	if (!empty(q))
	{
		return q->head->p;
	}
}

//Prepend to tail.
void enqueue(queue * q, process p)
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
}

//Pop from head
process dequeue(queue * q)
{
	if (!empty(q))
	{
		node * temp = q->head;
		process p = q->head->p;
		if (q->head == q->tail)
		{
			q->head = q->tail = NULL;
		} else {
			q->head = q->head->next;
		}
		temp->next = NULL;
		free(temp);
		return p;
	}

	process empty_queue;
	empty_queue.pid = -1;
	return empty_queue;
}

process min(queue * q, int min_field)
{
	node * curr = q->head;

	int min_time;
	if (min_field == SHORTEST)
	{
		min_time = curr->p.cpu_time_left;	
	}
	else if (min_field == EARLIEST)
	{
		min_time = curr->p.arrival_time;
	}

	process p;
	process nextp;
	process retp = curr->p;
	node * min_node = curr;
	node * next = curr->next;
	node * prev = NULL;

	while (curr)
	{

		if (next)
		{
			nextp = next->p;
			if (min_field == SHORTEST)
			{
				if (min_time > nextp.cpu_time_left)
				{
					min_time = nextp.cpu_time_left;
					retp = nextp;
					min_node = next;
					prev = curr;
				}	
			}
			else if (min_field == EARLIEST)
			{
				if (min_time > nextp.arrival_time || (min_time == nextp.arrival_time && min_node->p.pid > nextp.pid) )
				{
					min_time = nextp.arrival_time;
					retp = nextp;
					min_node = next;
					prev = curr;
				} 
			}
		}


		curr = curr->next;

		if (curr) 
		{
			next = curr->next;
		}
	}

	if (prev && min_node->p.pid != q->tail->p.pid)
	{
		prev->next = min_node->next;
	}
	else if (prev && min_node->p.pid == q->tail->p.pid)
	{
		prev->next = NULL;
		q->tail = prev;
	}
	else if (q->head == q->tail) 
	{
		q->head = q->tail = NULL;
	} else 
	{
		q->head = q->head->next;
	}
	free(min_node);
	return retp;
}

/*
int main(int argc, char **argv)
{
	queue q = init();
	process proc1;
	process proc2;
	process proc3;
	process proc4;
	proc1.arrival_time = 1;
	proc2.arrival_time = 2;
	proc3.arrival_time = 3;
	proc4.arrival_time = 4;
	proc1.cpu_time_left = 4;
	proc2.cpu_time_left = 5;
	proc3.cpu_time_left = 6;
	proc4.cpu_time_left = 1;
	proc1.pid = 1;
	proc2.pid = 2;
	proc3.pid = 3;
	proc4.pid = 4;


	enqueue(&q, proc1);
	enqueue(&q, proc2);
	enqueue(&q, proc3);
	process popped1 = dequeue(&q);
	process popped2 = dequeue(&q);

	printf("pop = %d", popped1.arrival_time);
	process peeked = peek(&q);
	printf("peek = %d\n", peeked.arrival_time);

	clear(&q);

	queue q2 = init();
	enqueue(&q2, proc1);
		printf("%d\n", q2.head);

	printf("%d\n", q2.tail);

	dequeue(&q2);
	printf("%d\n", q2.head);
	printf("%d\n", q2.tail);

	queue q3 = init();
	enqueue(&q3, proc3);
	enqueue(&q3, proc1);
	enqueue(&q3, proc2);
	enqueue(&q3, proc4);

	process peekedBeforeMin = peek(&q3);
	printf("peek = %d\n", peekedBeforeMin.cpu_time_left);

	process mini = min(&q3);
	printf("min 1 = %d %d\n", mini.cpu_time_left, mini.pid);
	mini = min(&q3);
	printf("min 2 = %d %d\n", mini.cpu_time_left, mini.pid);
	mini = min(&q3);
	printf("min 3 = %d %d\n", mini.cpu_time_left, mini.pid);
	mini = min(&q3);
	printf("min 4 = %d %d\n", mini.cpu_time_left, mini.pid);

	printf("is empty? %d\n", empty(&q3));

	enqueue(&q3, proc4);
	printf("is empty? %d\n", empty(&q3));
	find(&q3, 4);
}
*/






