#ifndef INCLUDED_MANAGER
#define INCLUDED_MANAGER
#include "process.h"
process set_process(int id);
action set_action(char type[10], int p_ind, int delay, int r_ind, int num_resources);
void fifo(process pool[], int resources[], int state[], int pool_sz, int resources_sz, FILE * inputf);
void bankers(process pool[], int resources[], int state[], int pool_sz, int resources_sz, FILE * inputf);
int safe_state(int pool_sz, int resources_sz, int state[], int current[pool_sz][resources_sz], int available[], int max[pool_sz][resources_sz]);
void summarize_results(char header[10], process pool[], int pool_sz);
#endif
