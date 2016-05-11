#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include "scheduler.h"
#include "queue.h"
#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define TERMINATED 3
#define UNSTARTED 4
#define RANDOM_INTS "random-numbers.txt"
#define MAX_INT 2 << 10

FILE * random_ints;
int verbose;

process init_process(int id, int a, int b, int c, int io) 
{
	process process;
	process.pid = id;
	process.arrival_time = a;
	process.cpu_burst_gen = b;
	process.cpu_burst = 0;
	process.cpu_time = c;
	process.cpu_time_left = process.cpu_time;
	process.io_burst_gen = io;
	process.io_burst = 0;

	process.time_blocked = 0;
	process.time_waited = 0;
	process.finished_at = 0;
	process.turnaround = 0;
	process.cpu_burst_remainder = 0;

	return process;
}

int process_cmp (const void *v1, const void *v2)
{
  const process *p1 = v1;
  const process *p2 = v2;
  int diff = p1->arrival_time - p2->arrival_time;

  /* Guarantees stable sort if arrival times are equal. */
  return (diff == 0) ? p1->pid - p2->pid : diff;
}



int main(int argc, char **argv)
{

	FILE * inputf;
	if (argc == 3)
	{
		verbose = 1;
		inputf = fopen(argv[2], "r");
	}
	else if (argc == 2)
	{
		verbose = 0;
		inputf = fopen(argv[1], "r");
	}

	random_ints = fopen(RANDOM_INTS, "r");
	int pool_sz;
	int a;
	int b;
	int c;
	int io;
	fscanf(inputf, "%d", &pool_sz);
	int p_ind;
	process pool[pool_sz];
	process pool_copy[pool_sz];
	process sorted_pool[pool_sz];
	process sorted_copy[pool_sz];
	process proc;


	int state[pool_sz];

	char * list_output;
	char * process_format = " ( %d %d %d %d )";
	printf("The original input was: %d", pool_sz);
	for (p_ind = 0; p_ind < pool_sz; p_ind ++)
	{
		fscanf(inputf, process_format, &a, &b, &c, &io);
		asprintf(&list_output, process_format, a, b, c, io);
		printf(list_output);
		proc = init_process(p_ind, a, b, c, io);
		pool[p_ind] = proc;	
		sorted_pool[p_ind] = proc;	
		state[p_ind] = UNSTARTED;
	}
	printf("\n");

	qsort(sorted_pool, pool_sz, sizeof(process), process_cmp);
	printf("The (sorted) input is: %d", pool_sz);
	for (p_ind = 0; p_ind < pool_sz; p_ind ++)
	{
		proc = sorted_pool[p_ind];
		asprintf(&list_output, process_format, proc.arrival_time, proc.cpu_burst_gen, proc.cpu_time_left, proc.io_burst_gen);
		printf(list_output);
	}
	printf("\n\n");


	list_output = NULL;
	process_format = NULL;

	memcpy(pool_copy, pool, sizeof(process) * pool_sz);
	memcpy(sorted_copy, sorted_pool, sizeof(process) * pool_sz);
	uni(state, pool_copy, sorted_copy, pool_sz);
	fclose(random_ints);
	random_ints = fopen(RANDOM_INTS, "r");


	int i = 0;
	for (i = 0; i < pool_sz; i ++)
	{
		state[i] = UNSTARTED;
	}	
	memcpy(pool_copy, pool, sizeof(process) * pool_sz);
	fcfs(state, pool_copy, pool_sz);
	fclose(random_ints);
	random_ints = fopen(RANDOM_INTS, "r");

	for (i = 0; i < pool_sz; i ++)
	{
		state[i] = UNSTARTED;
	}	
	memcpy(pool_copy, pool, sizeof(process) * pool_sz);
	sjf(state, pool_copy, pool_sz);
	fclose(random_ints);
	random_ints = fopen(RANDOM_INTS, "r");

	for (i = 0; i < pool_sz; i ++)
	{
		state[i] = UNSTARTED;
	}	
	memcpy(pool_copy, pool, sizeof(process) * pool_sz);
	roundrobin(2, state, pool_copy, pool_sz);

	fclose(random_ints);

	return 0;
}

void uni(int state[], process pool[], process sorted_pool[], int pool_sz)
{
	int term_count = 0;
	int p_ind;	
	int cycle = 0;
	process p;
	process * running = NULL;
	process * temp = NULL;

	double sum_cpu = 0;
	double sum_io = 0;

	queue ready = init();

	for (p_ind = 0; p_ind < pool_sz; p_ind ++)
	{
		temp = &sorted_pool[p_ind];
		temp->sorted_ind = p_ind;
	}

	temp = NULL;
	while (term_count < pool_sz)
	{

		if (verbose) print_prev_cycle(state, sorted_pool, pool_sz, cycle);

		if (running)
		{

			if (state[running->pid] == BLOCKED)
			{
				running->io_burst = running->io_burst - 1;
				running->time_blocked += 1;
				sum_io += 1;
				pool[running->pid] = *running;
				sorted_pool[running->sorted_ind] = *running;
			} 
			else if (state[running->pid] == RUNNING)
			{
				running->cpu_burst -= 1;
				running->cpu_time_left -= 1;
				pool[running->pid] = *running;
				sum_cpu += 1;
				sorted_pool[running->sorted_ind] = *running;
			}

			if (running->cpu_time_left == 0) {
				state[running->pid] = TERMINATED;
				term_count = term_count + 1;
				running->finished_at = cycle;
				running->turnaround = cycle - running->arrival_time;
				pool[running->pid] = *running;
				sorted_pool[running->sorted_ind] = *running;

				running = NULL;
			} 
			else if (running->cpu_burst == 0 && state[running->pid] == RUNNING)
			{
				running->io_burst = randomOS(running->io_burst_gen);
				state[running->pid] = BLOCKED;
				pool[running->pid] = *running;
				sorted_pool[running->sorted_ind] = *running;

			} 
			else if (running->io_burst == 0 && state[running->pid] == BLOCKED)
			{
				running->cpu_burst = randomOS(running->cpu_burst_gen);
				if (running->cpu_burst > running->cpu_time_left)
				{
					running->cpu_burst = running->cpu_time_left;
				}
				state[running->pid] = RUNNING;
				sorted_pool[running->sorted_ind] = *running;

				pool[running->pid] = *running;
			}
		}

		for (p_ind = 0; p_ind < pool_sz; p_ind ++)
		{
			temp = &sorted_pool[p_ind];

			if (temp->arrival_time > cycle) break;

			if (temp->arrival_time == cycle)
			{
				if (running) {
					state[temp->pid] = READY;
					temp->started_wait = cycle;
					pool[temp->pid] = *temp;
					sorted_pool[temp->sorted_ind] = *temp;

					enqueue(&ready, *temp);
				} 
				else 
				{
					running = temp;
					state[running->pid] = RUNNING;
					running->cpu_burst = randomOS(running->cpu_burst_gen);

					if (running->cpu_burst > running->cpu_time_left)
					{
						running->cpu_burst = running->cpu_time_left;
					}

					pool[running->pid] = *running;
					sorted_pool[running->sorted_ind] = *running;
				}
			}
		}	

		if (!empty(&ready))
		{
			if (!running) 
			{
				p = dequeue(&ready);	
				running = &p;
				running->time_waited = cycle - running->started_wait;
				state[running->pid] = RUNNING;
				running->cpu_burst = randomOS(running->cpu_burst_gen);

				if (running->cpu_burst > running->cpu_time_left)
				{
					running->cpu_burst = running->cpu_time_left;
				}

				pool[running->pid] = *running;
				sorted_pool[running->sorted_ind] = *running;

			}
		}
		cycle += 1;
	}

	temp = NULL;
	running = NULL;
	printf("The scheduling algorithm used was Uniprogrammed\n\n");
	print_stats(pool, pool_sz, cycle - 1, sum_cpu, sum_io);
	clear(&ready);
}

void fcfs(int state[], process pool[], int pool_sz)
{
	int term_count = 0;
	int p_ind;	
	int cycle = 0;
	process p;
	process * running = NULL;
	process * temp = NULL;

	queue ready = init();
	queue ties = init();
	int process_blocked = 0;

	double sum_cpu = 0;
	double sum_io = 0;

	//Maintain a sorted pool for verbose printout
	process sorted_pool[pool_sz];
	memcpy(sorted_pool, pool, sizeof(process) * pool_sz);
	qsort(sorted_pool, pool_sz, sizeof(process), process_cmp);

	while (term_count < pool_sz)
	{

		if (verbose) print_prev_cycle(state, sorted_pool, pool_sz, cycle);

		if (process_blocked)
		{
			process_blocked = 0;
			for (p_ind = 0; p_ind < pool_sz; p_ind ++) 
			{
				temp = &pool[p_ind];
				if (state[temp->pid] == BLOCKED)
				{
					process_blocked = 1;
					temp->io_burst = temp->io_burst - 1;
					temp->time_blocked += 1;
					if (temp->io_burst == 0)
					{
						state[temp->pid] = READY;
						temp->started_wait = cycle;
						enqueue(&ties, *temp);
					}
					sorted_pool[temp->pid] = *temp;
				}
			}

			if (process_blocked) sum_io += 1;
		}

		if (running) 
		{

			if (state[running->pid] == RUNNING)
			{
				running->cpu_burst = running->cpu_burst - 1;
				running->cpu_time_left = running->cpu_time_left - 1;
				sum_cpu += 1;
				sorted_pool[running->pid] = *running;
				pool[running->pid] = *running;
			}

			if (running->cpu_time_left == 0) {
				state[running->pid] = TERMINATED;
				term_count = term_count + 1;
				running->finished_at = cycle;
				running->turnaround = cycle - running->arrival_time;
				pool[running->pid] = *running;
				running = NULL;
			} 
			else if (running->cpu_burst == 0)
			{
				running->io_burst = randomOS(running->io_burst_gen);
				state[running->pid] = BLOCKED;
				sorted_pool[running->pid] = *running;
				pool[running->pid] = *running;
				running = NULL;
				process_blocked = 1;
			} 
		}

		for (p_ind = 0; p_ind < pool_sz; p_ind ++)
		{
			temp = &pool[p_ind];			

			if (temp->arrival_time == cycle)
			{
				state[temp->pid] = READY;
				temp->started_wait = cycle;
				enqueue(&ties, *temp);
			}
		}	

		break_ties(&ready, &ties);

		if (!empty(&ready) && !running)
		{
			p = dequeue(&ready);	
			running = &p;
			state[running->pid] = RUNNING;
			running->time_waited += cycle - running->started_wait;
			running->cpu_burst = randomOS(running->cpu_burst_gen);

			if (running->cpu_burst > running->cpu_time_left)
			{
				running->cpu_burst = running->cpu_time_left;
			}
			sorted_pool[running->pid] = *running;
			pool[running->pid] = *running;
		}
		cycle += 1;
	}


	temp = NULL;
	running = NULL;
	printf("The scheduling algorithm used was First Come First Served\n\n");
	print_stats(pool, pool_sz, cycle - 1, sum_cpu, sum_io);
	clear(&ready);
}

void sjf(int state[], process pool[], int pool_sz)
{
	int term_count = 0;
	int p_ind;	
	int cycle = 0;
	process p;
	process * running = NULL;
	process * temp = NULL;

	double sum_cpu = 0;
	double sum_io = 0;

	process sorted_pool[pool_sz];
	memcpy(sorted_pool, pool, sizeof(process) * pool_sz);
	qsort(sorted_pool, pool_sz, sizeof(process), process_cmp);

	queue ready = init();
	queue ties = init();
	int process_blocked = 0;
	while (term_count < pool_sz)
	{

		if (verbose) print_prev_cycle(state, sorted_pool, pool_sz, cycle);

		if (process_blocked)
		{
			process_blocked = 0;
			for (p_ind = 0; p_ind < pool_sz; p_ind ++) 
			{
				temp = &pool[p_ind];
				if (state[temp->pid] == BLOCKED)
				{
					process_blocked = 1;
					temp->io_burst = temp->io_burst - 1;
					temp->time_blocked += 1;
					if (temp->io_burst == 0)
					{
						state[temp->pid] = READY;
						temp->started_wait = cycle;
						enqueue(&ties, *temp);
					}
					sorted_pool[temp->pid] = *temp;
				}
			}

			if (process_blocked) sum_io += 1;
		}

		if (running) 
		{

			if (state[running->pid] == RUNNING)
			{
				running->cpu_burst = running->cpu_burst - 1;
				running->cpu_time_left = running->cpu_time_left - 1;
				sum_cpu += 1;
				sorted_pool[running->pid] = *running;
				pool[running->pid] = *running;
			}

			if (running->cpu_time_left == 0) {
				state[running->pid] = TERMINATED;
				term_count = term_count + 1;
				running->finished_at = cycle;
				running->turnaround = cycle - running->arrival_time;
				pool[running->pid] = *running;
				running = NULL;
			} 
			else if (running->cpu_burst == 0)
			{
				running->io_burst = randomOS(running->io_burst_gen);
				state[running->pid] = BLOCKED;
				sorted_pool[running->pid] = *running;
				pool[running->pid] = *running;
				running = NULL;
				process_blocked = 1;
			} 
		}

		for (p_ind = 0; p_ind < pool_sz; p_ind ++)
		{
			temp = &pool[p_ind];

			if (temp->arrival_time == cycle)
			{
					state[temp->pid] = READY;
					temp->started_wait = cycle;
					enqueue(&ties, *temp);
			}
		}	

		break_ties(&ready, &ties);

		if (!empty(&ready) && !running)
		{
			p = min(&ready, SHORTEST);
			running = &p;
			state[running->pid] = RUNNING;
			running->cpu_burst = randomOS(running->cpu_burst_gen);
			running->time_waited += cycle - running->started_wait;

			if (running->cpu_burst > running->cpu_time_left)
			{
				running->cpu_burst = running->cpu_time_left;
			}
			sorted_pool[running->pid] = *running;
			pool[running->pid] = *running;
		}
		cycle += 1;
	}


	temp = NULL;
	running = NULL;
	printf("The scheduling algorithm used was Shortest Job First\n\n");
	print_stats(pool, pool_sz, cycle - 1, sum_cpu, sum_io);
	clear(&ready);
}

void roundrobin(int interval_length, int state[], process pool[], int pool_sz)
{
	int term_count = 0;
	int p_ind;	
	int cycle = 0;
	process p;
	process * running = NULL;
	process * temp = NULL;
	//process * preempted = NULL;

	double sum_cpu = 0;
	double sum_io = 0;

	process sorted_pool[pool_sz];
	memcpy(sorted_pool, pool, sizeof(process) * pool_sz);
	qsort(sorted_pool, pool_sz, sizeof(process), process_cmp);


	queue ready = init();
	queue ties = init();
	int process_blocked = 0;
	while (term_count < pool_sz)
	{

		if (verbose) print_prev_cycle(state, sorted_pool, pool_sz, cycle);
		if (process_blocked)
		{
			process_blocked = 0;
			for (p_ind = 0; p_ind < pool_sz; p_ind ++) 
			{
				temp = &pool[p_ind];
				if (state[temp->pid] == BLOCKED)
				{
					process_blocked = 1;
					temp->io_burst = temp->io_burst - 1;
					temp->time_blocked += 1;
					if (temp->io_burst == 0)
					{
						state[temp->pid] = READY;
						temp->started_wait = cycle;
						enqueue(&ties, *temp);
					}
					sorted_pool[temp->pid] = *temp;
				}
			}

			if (process_blocked) sum_io += 1;
		}

		if (running)
		{
			if (state[running->pid] == RUNNING)
			{
				running->cpu_burst -= 1;
				running->cpu_time_left -= 1;
				sum_cpu += 1;
				sorted_pool[running->pid] = *running;
				pool[running->pid] = *running;
			}

			if (running->cpu_time_left == 0) {
				state[running->pid] = TERMINATED;
				term_count = term_count + 1;
				running->finished_at = cycle;
				running->turnaround = cycle - running->arrival_time;
				pool[running->pid] = *running;
				running = NULL;
			} 
			else if (running->cpu_burst == 0 && running->cpu_burst_remainder == 0)
			{
				running->io_burst = randomOS(running->io_burst_gen);
				state[running->pid] = BLOCKED;
				sorted_pool[running->pid] = *running;
				pool[running->pid] = *running;
				running = NULL;
				process_blocked = 1;
			}	
			else if (running->cpu_burst == 0 && running->cpu_burst_remainder != 0)
			{
				running->started_wait = cycle;
				state[running->pid] = READY;
				pool[running->pid] = *running;
				enqueue(&ties, *running);
				running = NULL;
			}					
		}

		for (p_ind = 0; p_ind < pool_sz; p_ind ++)
		{
			temp = &pool[p_ind];

			if (temp->arrival_time == cycle)
			{		
				state[temp->pid] = READY;
				temp->started_wait = cycle;
				enqueue(&ties, *temp);
			}
		}

		break_ties(&ready, &ties);

		if (!empty(&ready) && !running)
		{
			
				p = dequeue(&ready);
				running = &p;
				running->time_waited += cycle - running->started_wait;	
				state[running->pid] = RUNNING;

				if (!running->cpu_burst_remainder)
				{
					//Get a new cpu_burst
					running->cpu_burst_remainder = randomOS(running->cpu_burst_gen);
				}

				if (running->cpu_burst_remainder > interval_length)
				{
					running->cpu_burst_remainder -= 2;
					running->cpu_burst = 2;
				} 
				else
				{
					running->cpu_burst = running->cpu_burst_remainder;
					running->cpu_burst_remainder = 0;
				}

				if (running->cpu_burst > running->cpu_time_left)
				{
					running->cpu_burst_remainder = 0;
					running->cpu_burst = running->cpu_time_left;
				}
				sorted_pool[running->pid] = *running;
				pool[running->pid] = *running;			
		}
				
		cycle += 1;
	}


	temp = NULL;
	running = NULL;
	printf("The scheduling algorithm used was Round Robin, with q = 2\n\n");
	print_stats(pool, pool_sz, cycle - 1, sum_cpu, sum_io);
	clear(&ready);
}

void break_ties(queue * ready, queue * ties)
{
	process p;
	while (!empty(ties))
	{
		p = min(ties, EARLIEST);
		enqueue(ready, p);
	}
}


/*
void create_processes(int state[], process pool[], queue ready, int pool_sz, int cycle, process * running, FILE * random_ints)
{
	int p_ind;
	process * temp;
	for (p_ind = 0; p_ind < pool_sz; p_ind ++)
	{
		temp = &pool[p_ind];

		if (temp->arrival_time > cycle) {
			break;
		}

		if (temp->arrival_time == cycle)
		{
			if (running) {
				state[temp->pid] = READY;
				enqueue(&ready, *temp);
			} else {
				running = temp; //running = &pool[p_ind]; //
				state[running->pid] = RUNNING;
				running->cpu_burst = 1 + (randomos(random_ints)) % (running->cpu_burst_gen);
				if (running->cpu_burst > running->cpu_time_left)
				{
					running->cpu_burst = running->cpu_time_left;
				}
			}
		}
	}
}
*/



long randomOS(int max)
{
	if (!feof(random_ints)) {
		long ran = 0;
		fscanf(random_ints, "%d\n", &ran);	
		return 	1 + (ran) % (max);	
	} else {
		return -1;
	}
}

void print_prev_cycle(int state[], process pool[], int pool_sz, int cycle) {
	int p_ind = 0;
	process p;
	printf("Before cycle %10d:", cycle);
	for (p_ind = 0; p_ind < pool_sz; p_ind ++)
	{
		p = pool[p_ind];
		print_state(state, p);			
	}
	printf(".\n");
}

void print_state(int state[], process p) 
{
	int st = state[p.pid];

	switch(st) {
		case 0:
			printf("  %10s%4d", "ready", 0);
			break;
		case 1:
			printf("  %10s%4d", "running", p.cpu_burst);
			break;
		case 2:
			printf("  %10s%4d", "blocked", p.io_burst);
			break;
		case 3:
			printf("  %10s%4d", "terminated", 0);
			break;
		case 4:
			printf("  %10s%4d", "unstarted", 0);
			break;
		default:
			break;
	}
}

void print_stats(process pool[], int pool_sz, int finished_at, double sum_cpu, double sum_io)
{
	process p;
	int i;

	double mean_turnaround = 0;
	double mean_waiting = 0;

	for (i = 0; i < pool_sz; i ++)
	{
		p = pool[i];
		mean_turnaround += p.turnaround;
		mean_waiting += p.time_waited;
		printf("Process %d:\n\t(A,B,C,IO) = (%d,%d,%d,%d)\n\tFinishing time: %d\n\tTurnaround time: %d\n\tI/O time: %d\n\tWaiting time: %d\n\n", p.pid, p.arrival_time, p.cpu_burst_gen, p.cpu_time, p.io_burst_gen, p.finished_at, p.turnaround, p.time_blocked, p.time_waited);
	}

	mean_turnaround /= pool_sz;
	mean_waiting /= pool_sz;
	printf("Summary Data:\n\t Finishing time: %d\n\t CPU Utilization: %f\n\t I/O Utilization: %f\n\t Throughput: %f processes per hundred cycles\n\t Average turnaround time: %.6f\n\t Average waiting time: %.6f\n\n",
		   finished_at, sum_cpu / finished_at, sum_io / finished_at, pool_sz / ((finished_at / 100.0)), mean_turnaround, mean_waiting);
}
