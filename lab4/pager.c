#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>
#include "pager.h"
#include "process.h"
#include "page.h"
#define ALIVE 1
#define DEAD 0
#define FREE -1
#define MAX_INT 2147483647
#define LRU "lru"
#define FIFO "fifo"
#define RAND "random"
#define RANDOM_INTS "random-numbers.txt"

FILE * random_ints;

process * init_processes(int job_mix, int pool_sz) {
	int i;
	process * pool = (process *) malloc(sizeof(process) * pool_sz);
	if (job_mix == 1) {
		process p;
		p.pid = 0;
		p.probA = 1;
		p.probB, p.probC = 0;
		pool[0] = p;

	} else if (job_mix == 2) {
		for (i = 0; i < 4; i ++) {
			process p;
			p.pid = i;
			p.probA = 1;
			p.probB = 0;
			p.probC = 0;
			pool[i] = p;
		}

	} else if (job_mix == 3) {
		for (i = 0; i < 4; i ++) {
			process p;
			p.pid = i;
			p.probA = .0, p.probB = .0, p.probC = .0;
			pool[i] = p;
		}

	} else if (job_mix == 4) {
		for (i = 0; i < 4; i ++) {
			process p;
			p.pid = i;
			p.probA = (i == 3) ? .5 : .75;
			p.probB = .125;
			p.probC = .125;
			if (i == 0) {
				p.probB = .25;
				p.probC = .0;
			} else if (i == 1) {
				p.probB = .0;
				p.probC = .25;
			}
			pool[i] = p;
		}
	}

	return pool; 
}

pager * init_pager(process * pool, int pool_sz, int machine_sz, int page_sz, int process_sz, char * rp_algo, int debug) {
	pager * manager = (pager *) malloc(sizeof(pager));
	manager->pool = (process *) malloc(sizeof(process) * pool_sz);
	memcpy(manager->pool, pool, sizeof(process) * pool_sz);
	free(pool);
	manager->pool_sz = pool_sz;
	manager->machine_sz = machine_sz;
	manager->page_sz = page_sz;
	manager->process_sz = process_sz;
	manager->rp_algo = rp_algo;
	manager->frame_ct = machine_sz / page_sz;
	manager->ftable = (page *) malloc(sizeof(page) * (manager->frame_ct));
	manager->ttable = (int *) malloc(sizeof(int) * (manager->frame_ct));
	manager->page_queue = (queue *) malloc(sizeof(queue));
	manager->highest_free_frame_ind = (manager->frame_ct) - 1;
	manager->debug = debug;

	int num_addressable_pages = process_sz / page_sz; //There are w addressable words in a process, so there are w / pz addressable pages.
													  // Our machine might not fit be able to fit all w / pz pages.
	*(manager->page_queue) = init(num_addressable_pages, manager->frame_ct);



	int f;
	page free;
	free.owner_pid = -1;
	free.page_num = -1;
	for (f = 0; f < manager->frame_ct; f ++) {
		manager->ftable[f] = free;
	}

	return manager;
}

void destroy_pager(pager * manager) {
	free(manager->pool);
	free(manager->ftable);
	free(manager->ttable);
	clear(manager->page_queue);
	free(manager->page_queue);
	free(manager);
}

int main(int argc, char **argv)
{

	int machine_sz;
	int page_sz;
	int process_sz;
	int job_mix;
	int num_refs;
	char * replacement_algo;
	int k;

	int debug = 0;
	random_ints = fopen(RANDOM_INTS, "r");

	if (argc < 7 || argc > 8)
	{
		printf("Incorrect input.");
		exit(0);
	} 
	else
	{
		machine_sz = atoi(argv[1]);
		page_sz = atoi(argv[2]);
		process_sz = atoi(argv[3]);
		job_mix = atoi(argv[4]);
		num_refs = atoi(argv[5]);
		replacement_algo = argv[6];

		if (argc == 8) {
			debug = 1;
		}
	}

	int pool_sz = (job_mix == 1) ? 1 : 4;
	process * pool = init_processes(job_mix, pool_sz);

	int i;
	for (i = 0; i < pool_sz; i ++) {
		int k = i + 1;
		pool[i].curr_reference = (111 * k) % process_sz;
		pool[i].num_references = num_refs;
		pool[i].residency = 0;
		pool[i].evicted = 0;
		pool[i].faults = 0;
	}

	pager * p = init_pager(pool, pool_sz, machine_sz, page_sz, process_sz, replacement_algo, debug);

	echo_input(p, job_mix, num_refs, debug);

	pager man = *p;
	pager_routine(p);

	print_stats(p->pool, p->pool_sz);
	fclose(random_ints);
	destroy_pager(p);

	return 0;
}

void pager_routine(pager * manager) {

	int term_count = 0;
	int cycle = 1;

	int rr_timer = 3;
	int ref_count;
	int pid;
	int curr_word;
	process * curr_process;

	int page_sz = manager->page_sz;
	int pool_sz = manager->pool_sz;
	int process_sz = manager->process_sz;

	
	while (term_count != pool_sz) {

		for (pid = 0; pid < pool_sz; pid ++) {
			curr_process = &(manager->pool[pid]);
			for (ref_count = 0; ref_count < 3 && curr_process->num_references > 0; ref_count ++) {
				curr_word = curr_process->curr_reference;
				simulate_step(manager, curr_process, curr_word, cycle);
				if (curr_process->num_references == 0) {
					term_count += 1;
				}

				cycle++;
				
			}
		}
	}
}

void simulate_step(pager * manager, process * curr_process, int curr_word, int cycle) {
	char * algo = manager->rp_algo;

	int process_sz = manager->process_sz;
	int page_sz = manager->page_sz;
	int ref_page = curr_word / page_sz;
	int debug = manager->debug;


	process p = (*curr_process);

	int f;
	int hit = 0;

	int event = 2;
	//Used for LRU eviction policy. -1 <=> no entry in map and map is full, 0 <=> free frame for new entry, 1 => hit in map. 
	//_ => non-LRU policy to be used.


	for (f = 0; f < manager->frame_ct && (hit != 1); f ++) {
		if (manager->ftable[f].page_num == ref_page && manager->ftable[f].owner_pid == p.pid) {
			hit = 1;
		}
	}

	page new_page;
	new_page.owner_pid = p.pid;
	new_page.page_num = ref_page;

	if (!strcmp(algo, LRU)) {
		event = reference_page(manager->page_queue, new_page);
	}

	if (!hit || event <= 0) {
			//miss

			manager->pool[p.pid].faults += 1;
			if (manager->highest_free_frame_ind < 0) {
				//evict
				page evicted;
				if (!strcmp(algo, LRU)) {
					evicted = dequeue(manager->page_queue);

				} else if (!strcmp(algo, FIFO)) {
					evicted = dequeue(manager->page_queue);

				} else if (!strcmp(algo, RAND)) {
					evicted = manager->ftable[randomOS() % manager->frame_ct];
				} else {
					printf("No match for input algorithm %s\n", algo);
					return;
				}

				int evicted_f;
				for (f = 0; f < manager->frame_ct; f ++) {
					if (manager->ftable[f].page_num == evicted.page_num && manager->ftable[f].owner_pid == evicted.owner_pid) {
						evicted_f = f;
						break;
					}
				}
				//manager->ftable[evicted.frame_ind] = -1;
				int loaded = manager->ttable[evicted_f];
				int residency = cycle - loaded;
				manager->pool[evicted.owner_pid].residency += residency;
				manager->pool[evicted.owner_pid].evicted += 1;

				if (debug) {
					printf("%d references word %d (page %d) at time %d: Fault, evicting page %d of %d from frame %d.\n",
						p.pid + 1, curr_word, ref_page, cycle, evicted.page_num, evicted.owner_pid + 1, evicted_f);
				}
				

				manager->ftable[evicted_f].page_num = ref_page;
				manager->ftable[evicted_f].owner_pid = p.pid;
				manager->ttable[evicted_f] = cycle;
			} else {
				int free_frame = manager->highest_free_frame_ind;
				manager->ftable[free_frame].page_num = ref_page;
				manager->ftable[free_frame].owner_pid = p.pid;
				manager->ttable[free_frame] = cycle;
				manager->highest_free_frame_ind -= 1;
				if (debug) {
					printf("%d references word %d (page %d) at time %d: Fault, using free frame %d.\n",
						p.pid + 1, curr_word, ref_page, cycle, free_frame);
				}
			}
 
			if (!strcmp(algo, FIFO)) {
				enqueue(manager->page_queue, new_page);
			} else if (!strcmp(algo, LRU)) {
				reference_page(manager->page_queue, new_page);
			}
		} 

	

	if (hit && debug) {
		printf("%d references word %d (page %d) at time %d: Hit in frame %d.\n",
						p.pid + 1, curr_word, ref_page, cycle, f - 1);
	}
	

	curr_process->curr_reference = get_next_reference((*curr_process), process_sz);
	curr_process->num_references -= 1;
	manager->pool[curr_process->pid] = (*curr_process);



}

int randomOS()
{
	if (!feof(random_ints)) {
		int ran;
		fscanf(random_ints, "%d\n", &ran);
		return 	ran;
	} else {
		return -1;
	}
}

int get_next_reference(process p, int process_sz) {

	double ran = randomOS() / (MAX_INT + 1.0);
	int current_ref = p.curr_reference;
	int next_ref;
	if (ran < p.probA) {
		next_ref = (current_ref + 1) % process_sz;
	} else if (ran < p.probA + p.probB) {
		next_ref = (current_ref - 5 + process_sz) % process_sz;
	} else if (ran < p.probA + p.probB + p.probC) {
		next_ref = (current_ref + 4) % process_sz;
	} else {
		int new_ran = (randomOS()) % process_sz;
		next_ref = new_ran;
	}

	return next_ref;
}

void echo_input(pager * manager, int job_mix, int num_references, int debug) {
	printf("The machine size is %d.\n"
		   "The page size is %d.\n"
		   "The process size is %d.\n"
		   "The job mix number is %d.\n"
		   "The number of references per process is %d.\n"
		   "The replacement algorithm is %s.\n", manager->machine_sz, manager->page_sz, manager->process_sz,
		    job_mix, num_references, manager->rp_algo);
	if (debug) {
		printf("The level of debugging output is %d.\n\n", debug);
	} else {
		printf("\n");
	}
}



void print_stats(process * pool, int pool_sz) {
	int i;
	printf("\n");
	int total_faults = 0;
	double total_mean_residency = 0.0;
	int total_evicted = 0;
	double mean_residency;
	int count = 0;
	for (i = 0; i < pool_sz; i ++) {
		if (pool[i].evicted) {
			mean_residency = (1.0 * pool[i].residency) / pool[i].evicted;
			total_mean_residency += pool[i].residency;
			count += pool[i].evicted;
		}
		total_faults += pool[i].faults;

		if (pool[i].evicted) {
			printf("Proccess %d had %d faults and %f average residency.\n", pool[i].pid + 1, pool[i].faults, mean_residency);
		} else {
			printf("Proccess %d had %d faults.\n\t With no evictions, the average residence is undefined.", pool[i].pid + 1, pool[i].faults);
		}
	}

	if (count) {
		printf("\nThe total number of faults is %d and the overall average residency is %f.\n\n",
			total_faults,
			total_mean_residency / count);
	} else {
		printf("\nThe total number of faults is %d.\n\t With no evictions, the overall average residence is undefined.\n", total_faults);
	}
	
}




