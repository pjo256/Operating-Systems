#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include "queue.h"

queue init(int num_addressable_pages, int max_resident_pages)
{
	queue q;
	q.head = NULL;
	q.tail = NULL;
	q.size = 0;
	q.max_residents = max_resident_pages;

	map page_mapping;
	page_mapping.table = (node **) malloc(sizeof(node *) * num_addressable_pages * 4);
	page_mapping.max_size = num_addressable_pages * 4;


	int i;
	for (i = 0; i < num_addressable_pages * 4; i ++) {
		page_mapping.table[i] = NULL;
	}
	q.page_to_node = page_mapping;


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

	free(q->page_to_node.table);
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

page * peek(queue * q)
{
	if (!empty(q))
	{
		return &(q->head->p);
	} 
}

//Prepend to tail.
void enqueue(queue * q, page p)
{
	node * temp = (node *) malloc (sizeof(node));
	temp->next = temp->previous = NULL;
	temp->p = p;

	if (q->tail != NULL) 
	{
		temp->previous = q->tail;
		q->tail->next = temp;
		q->tail = temp;
	} 
	else
	{
		q->head = q->tail = temp;
	}

	q->size += 1;

	map assoc_array = q->page_to_node;
	int index = (assoc_array.max_size / 4) * p.owner_pid + p.page_num;
	//printf("PTE index is %d\n.", index);
	if (assoc_array.table[index] == NULL) {
		//printf("Page was null");
		assoc_array.table[index] = temp;
	}
	//printf("Page number enqueued is %d\n.", p.page_num);
}

//Pop from head
page dequeue(queue * q)
{
	if (!empty(q))
	{
		node * temp = q->head;
		page p = q->head->p;
		if (q->head == q->tail)
		{
			q->head = q->tail = NULL;
		} else {

			q->head = q->head->next;
			q->head->previous = NULL;
		}
		temp->next = NULL;
		free(temp);
		q->size -= 1;
		int index = (q->page_to_node.max_size / 4) * p.owner_pid + p.page_num;
		q->page_to_node.table[index] = NULL;
		return p;
	}

	page empty_queue;
	empty_queue.owner_pid = -1;
	return empty_queue;
}

//Decided to move this here seeing as this queue implementation is not generic
int reference_page(queue * q, page p) {
	map assoc_array = q->page_to_node;

	int i = 0;
	for (i = 0; i < assoc_array.max_size; i ++) {
		//printf("Location %d contains %p\n", i, (void *) assoc_array.table[i]);
	}


	int num_addressable_pages = assoc_array.max_size / 4;
	int index = num_addressable_pages * p.owner_pid + p.page_num;
	node * page_node = assoc_array.table[index];
	//printf("Referenced PTE %d\n", index);


	if (page_node == NULL && q->max_residents == q->size) {
		//printf("Table size %d vs. queue size %d\n", assoc_array.max_size, q->size);
		//Need to evict
		return -1;
	} else if (page_node == NULL) {
		//Fault, add to free frame (there exists at least one)
		//printf("Page node was NULL\n");
		enqueue(q, p);
		return 0;
	} else if (q->head != q->tail && page_node != q->tail) {
		//printf("Corresponding node has page number %d\n.", page_node->p.page_num);

		//Hit, multiple references in queue

		//Move node to tail.
		node * next_node = page_node->next;
		node * prev_node = page_node->previous;
		

		if (prev_node == NULL) {
			//Node is at head
			node * new_head = q->head->next;
			new_head->previous = NULL;
			q->head = new_head;
		} else {
			next_node->previous = prev_node;
			prev_node->next = next_node;
		}

		page_node->next = NULL;
		page_node->previous = q->tail;
		q->tail->next = page_node;
		q->tail = page_node;

		return 1;
	} else if (q->head == q->tail || page_node == q->tail) {
		//Head == tail, no replacement in queue
		//printf("Corresponding node has page number %d\n.", page_node->p.page_num);
		return 1;
	}


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







