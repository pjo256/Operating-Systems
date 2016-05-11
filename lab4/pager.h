#ifndef INCLUDED_PAGER
#define INCLUDED_PAGER
#include "process.h"
#include "page.h"
#include "queue.h"
typedef struct pager {

	process * pool;
	int pool_sz;
	int frame_ct;
	page * ftable;
	int * ttable;
	int highest_free_frame_ind;
	int machine_sz;
	int page_sz;
	int process_sz; 
	int debug;
	char * rp_algo;

	queue * page_queue;
} pager;
process * init_processes(int job_mix, int pool_sz);
int randomOS();
int get_next_reference(process p, int process_sz);
void pager_routine(pager * manager);
void simulate_step(pager * manager, process * current_process, int current_word, int cycle);
pager * init_pager(process * pool, int pool_sz, int machine_sz, int page_sz, int process_sz, char * rp_algo, int debug);
void destroy_pager(pager * manager);
void echo_input(pager * manager, int job_mix, int num_references, int debug);
void print_stats(process * pool, int pool_sz);
#endif
