#ifndef INCLUDED_SCHEDULER
#define INCLUDED_SCHEDULER
#include "process.h"
#include "queue.h"
process init_process(int id, int a, int b, int c, int io); 
int process_cmp (const void *v1, const void *v2);
long randomOS(int max);
//void create_processes(int state[], process pool[], queue ready, int pool_sz, int cycle, process * running, FILE * random_ints);
void uni(int state[], process pool[], process sorted_pool[], int pool_sz);
void fcfs(int state[], process pool[], int pool_sz);
void sjf(int state[], process pool[], int pool_sz);
void roundrobin(int interval_length, int state[], process pool[], int pool_sz);
void break_ties(queue * ready, queue * ties);
void rr_break_ties(int state[], queue * ready, queue * ties, process * preempted);
void print_prev_cycle(int state[], process pool[], int pool_sz, int cycle);
void print_state(int state[], process p);
void print_stats(process pool[], int pool_sz, int finished_at, double sum_cpu, double sum_io);
#endif
