#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include "manager.h"
#include "queue.h"
#define ALIVE 1
#define DEAD 0
#define MAX_INT 2 << 10

process init_process(int id) 
{
	process process;
	process.pid = id;
	process.time_waited = 0;
	process.finished_at = 0;
	process.delay = 0;
	process.aborted = 0;
	process.blocked = 0;
	process.handled = 0;

	process.request_release = init();
	return process;
}

action init_action(char type[10], int p_ind, int delay, int r_ind, int num_resources) 
{
	action action;

	if (!strcmp(type, "initiate")) {
		action.type = INIT;
	} else if (!strcmp(type, "request")) {
		action.type = REQ;
	} else if (!strcmp(type, "release")) {
		action.type = REL;
	} else {
		action.type = TERM;
	}

	action.delay = delay;
	action.r_ind = r_ind;
	action.p_ind = p_ind;
	action.num_resources = num_resources;

	return action;
}

int main(int argc, char **argv)
{

	FILE * inputf;

	if (argc == 2)
	{
		inputf = fopen(argv[1], "r");
	} 
	else 
	{
		printf("Enter an input file name.");
		exit(0);
	}

	int pool_sz;
	int resource_types;
	int resources_per_type;

	fscanf(inputf, "%d %d", &pool_sz, &resource_types);
	int resources[resource_types];
	int i;
	for (i = 0; i < resource_types; i ++) {
		fscanf(inputf, "%d", &resources_per_type);
		resources[i] = resources_per_type;
	}
	
	int p_ind;
	int r_ind;
	process pool[pool_sz];

	process proc;


	int state[pool_sz];

	for (p_ind = 0; p_ind < pool_sz; p_ind ++)
	{
		proc = init_process(p_ind);
		pool[p_ind] = proc;	
		state[p_ind] = ALIVE;
	}

	for (r_ind = 0; r_ind < resource_types; r_ind ++) 
	{
		resources[r_ind] = resources_per_type;
	}

	process pool_copy[pool_sz];
	int resources_copy[resource_types];
	int state_copy[pool_sz];

	memcpy(pool_copy, pool, sizeof(process) * pool_sz);
	memcpy(resources_copy, resources, sizeof(int) * resource_types);
	memcpy(state_copy, state, sizeof(int) * pool_sz);
	fifo(pool_copy, resources_copy, state_copy, pool_sz, resource_types, inputf);

	fclose(inputf);
	inputf = fopen(argv[1], "r");
	fscanf(inputf, "%d %d", &pool_sz, &resource_types);
	for (i = 0; i < resource_types; i ++) {
		fscanf(inputf, "%d", &resources_per_type);
		resources[i] = resources_per_type;
	}

	memcpy(pool_copy, pool, sizeof(process) * pool_sz);
	memcpy(resources_copy, resources, sizeof(int) * resource_types);
	memcpy(state_copy, state, sizeof(int) * pool_sz);
	bankers(pool_copy, resources_copy, state_copy, pool_sz, resource_types, inputf);

	fclose(inputf);

	return 0;
}

void fifo(process pool[], int resources[], int state[], int pool_sz, int resources_sz, FILE * inputf) { 
	int term_count = 0;

	enum action_type type;
	char descript[10];
	int p_ind;
	int r_ind;
	int delay;
	int num_resources;
	int num_delayed;

	int available_next_cycle[resources_sz];
	int cycle = 0;
	int deadlock;
	int next_abort = 0;

	int used_resources[resources_sz][pool_sz];
	int i;

	memset(available_next_cycle, 0, sizeof(int) * resources_sz);

	for (i = 0; i < resources_sz; i ++) {
		memset(used_resources[i], 0, sizeof(int) * pool_sz);
	}

	action activity;

	while (fscanf(inputf, "%s %d %d %d %d", &descript, &p_ind, &delay, &r_ind, &num_resources) != EOF) {
		activity = init_action(descript, p_ind - 1, delay, r_ind - 1, num_resources);
		enqueue(&(pool[p_ind - 1].request_release), activity);
	}


	//Queue of all incomplete requests
	queue pending_requests = init();


	while (term_count < pool_sz) {

		deadlock = 1;

		//No deadlock if any process is delayed, i.e. busy computing.
		num_delayed = 0;

		//Prevents processes from being processed twice per cycle
		for (i = 0; i < pool_sz; i ++) {
			pool[i].handled = 0;
		}

		for (i = 0; i < resources_sz; i ++) {
			resources[i] += available_next_cycle[i];
			available_next_cycle[i] = 0;
		}

		int elements_seen = 0;
		int q_size = pending_requests.size;

		//Examine all blocked requests, and see if any request can be resolved. 
		//Note that all requests in this queue no longer have a delay since requests
		//are only enqueued when a process is not delayed.
		while (elements_seen < q_size) {

			action pending_request = dequeue(&pending_requests);
			if (resources[pending_request.r_ind]  - pending_request.num_resources >= 0) {
				resources[pending_request.r_ind] -= pending_request.num_resources;
				used_resources[pending_request.r_ind][pending_request.p_ind] += pending_request.num_resources;
				deadlock = 0;
				pool[pending_request.p_ind].delay = pending_request.delay;
				pool[pending_request.p_ind].blocked = 0;
				pool[pending_request.p_ind].handled = 1;
			} else {
				enqueue(&pending_requests, pending_request);
				pool[pending_request.p_ind].time_waited += 1;
			}
			

			elements_seen += 1;
		}	

		//Handle a request, release, or terminate for a non-delayed process,
		for (i = 0; i < pool_sz; i ++) {

			//but don't handle a process twice per cycle.
			if (state[i] != ALIVE || pool[i].handled) continue;

			if (pool[i].delay == 0 && pool[i].blocked == 0) {

				action * a_ptr =  peek(&(pool[i].request_release));

				//Process is busy computing, not ready for another activity.
				if (a_ptr->delay > 0) {
					a_ptr->delay -= 1;
					num_delayed += 1;
				} else {

					activity = dequeue(&(pool[i].request_release));
					type = activity.type;
					if (type == INIT) {
						state[i] = ALIVE;
						deadlock = 0;
					} else if (type == REQ) {
						if (resources[activity.r_ind] - activity.num_resources >= 0) {
							resources[activity.r_ind] -= activity.num_resources;
							used_resources[activity.r_ind][i] += activity.num_resources;
							deadlock = 0;
							pool[i].blocked = 0;
							pool[i].delay = activity.delay;
						} else {
							enqueue(&pending_requests, activity);
							pool[i].blocked = 1;
							pool[i].time_waited += 1;
						}
					} else if (type == REL) { 			
						
						available_next_cycle[activity.r_ind] += activity.num_resources;
						used_resources[activity.r_ind][i] -= activity.num_resources;
						pool[i].delay = activity.delay;
						deadlock = 0;
						
					} else if (type == TERM) {
						used_resources[activity.r_ind][i] = 0;
						state[i] = DEAD;
						term_count += 1;
						pool[i].finished_at = cycle;
						deadlock = 0;
					} else {
						printf("Invalid activity in input.");
						exit(0);
					}
				}
			} else if (pool[i].blocked == 0) {
				pool[i].delay -= 1;
				num_delayed += 1;
			} 
		}			

		//Release resources of lowest numbered process and determine whether deadlock persists.
		while (deadlock && (num_delayed == 0)) {
			printf("Deadlock detected: aborting lowest numbered task and releasing resources for next cycle.\n");	
			state[next_abort] = DEAD;
			term_count += 1;
			pool[next_abort].aborted = 1;
			pool[next_abort].finished_at = cycle;
			for (i = 0; i < resources_sz; i ++) {
				available_next_cycle[i] += used_resources[i][next_abort];
				used_resources[i][next_abort] = 0;
			}

			next_abort += 1;

			while (next_abort < pool_sz && state[next_abort] == DEAD) {
				next_abort += 1;
			}

			int elements_seen = 0;
			int q_size = pending_requests.size;
			while (elements_seen < q_size) {
				action pending_request = dequeue(&pending_requests);
				if (pool[pending_request.p_ind].aborted) continue;
				
				if ((resources[pending_request.r_ind] + available_next_cycle[pending_request.r_ind]) - pending_request.num_resources >= 0) {
					deadlock = 0;
				} 
				
				enqueue(&pending_requests, pending_request);	
				elements_seen += 1;
			}	

		}
		cycle ++;
	}

	summarize_results("FIFO", pool, pool_sz);
}

void bankers(process pool[], int resources[], int state[], int pool_sz, int resources_sz, FILE * inputf) { 
	int term_count = 0;

	int max[pool_sz][resources_sz]; //Initial claims
	int current[pool_sz][resources_sz]; //Current resources, mapping from pid to resource id.
	int used_resources[resources_sz][pool_sz]; //Reverse mapping
	int requested[pool_sz][resources_sz]; //Number of resources requested since last release.

	enum action_type type;
	char descript[10];
	int p_ind;
	int r_ind;
	int delay;
	int num_resources;
	int num_delayed;

	int available_next_cycle[resources_sz];
	int cycle = 0;
	int deadlock;
	int next_abort = 0;

	int i;

	memset(available_next_cycle, 0, sizeof(int) * resources_sz);

	for (i = 0; i < resources_sz; i ++) {
		memset(used_resources[i], 0, sizeof(int) * pool_sz);
	}

	action activity;

	while (fscanf(inputf, "%s %d %d %d %d", &descript, &p_ind, &delay, &r_ind, &num_resources) != EOF) {
		activity = init_action(descript, p_ind - 1, delay, r_ind - 1, num_resources);

		if (!strcmp(descript, "initiate")) {
			if (num_resources > resources[r_ind - 1]) {
				pool[p_ind - 1].aborted = 1;
				state[p_ind - 1] = DEAD;
				term_count ++;
				printf("Banker aborts task %d before run begins:\n\tclaim for resource %d (%d) exceeds number of units present (%d)\n",
						p_ind, r_ind, num_resources, resources[r_ind - 1]);
			} else {
				max[p_ind - 1][r_ind - 1] = num_resources;
				current[p_ind - 1][r_ind - 1] = 0;
				requested[p_ind - 1][r_ind - 1] = 0;
			}
		}
		enqueue(&(pool[p_ind - 1].request_release), activity);
	}

	queue pending_requests = init();


	while (term_count < pool_sz) {

		//No deadlock if any process is delayed, i.e. busy computing.
		num_delayed = 0;

		//Prevents processes from being processed twice per cycle
		for (i = 0; i < pool_sz; i ++) {
			pool[i].handled = 0;
		}

		for (i = 0; i < resources_sz; i ++) {
			resources[i] += available_next_cycle[i];
			available_next_cycle[i] = 0;
		}

		int elements_seen = 0;
		int q_size = pending_requests.size;

		//Examine all blocked requests, and see if any request can be resolved. 
		//Note that all requests in this queue no longer have a delay since requests
		//are only enqueued when a process is not delayed.
		while (elements_seen < q_size) {

			//Only dequeue if all requested resources can be granted.
			action pending_request = dequeue(&pending_requests);

			int req = pending_request.num_resources;
			requested[pending_request.p_ind][pending_request.r_ind] += req;

			//Pending request breaks initial claim.
			if (req > max[pending_request.p_ind][pending_request.r_ind] || 
				requested[pending_request.p_ind][pending_request.r_ind] > max[pending_request.p_ind][pending_request.r_ind]) {

				state[pending_request.p_ind] = DEAD;
				term_count += 1;
				pool[pending_request.p_ind].aborted = 1;
				pool[pending_request.p_ind].finished_at = cycle;
				for (i = 0; i < resources_sz; i ++) {
					available_next_cycle[i] += used_resources[i][pending_request.p_ind];
					used_resources[i][pending_request.p_ind] = 0;
				}	
			} else {

				current[pending_request.p_ind][pending_request.r_ind] += pending_request.num_resources;
				resources[pending_request.r_ind] -= pending_request.num_resources;

				//Attempt to resolve pending request.
				if (resources[pending_request.r_ind] < 0 || !safe_state(pool_sz, resources_sz, state, current, resources, max)) {
					current[pending_request.p_ind][pending_request.r_ind] -= pending_request.num_resources;
					resources[pending_request.r_ind] += pending_request.num_resources;
					requested[pending_request.p_ind][pending_request.r_ind] -= pending_request.num_resources;

					enqueue(&pending_requests, pending_request);
					pool[pending_request.p_ind].time_waited += 1;
					pool[pending_request.p_ind].blocked = 1;
				} else {
					used_resources[pending_request.r_ind][pending_request.p_ind] += pending_request.num_resources;
					pool[pending_request.p_ind].delay = pending_request.delay;
					pool[pending_request.p_ind].blocked = 0;
					pool[pending_request.p_ind].handled = 1;
				}
			}		

			elements_seen += 1;
		}	

		//Handle a request, release, or terminate for a non-delayed process.
		for (i = 0; i < pool_sz; i ++) {

			if (state[i] != ALIVE || pool[i].handled) continue;

			if (pool[i].delay == 0 && pool[i].blocked == 0) {

				action * a_ptr =  peek(&(pool[i].request_release));

				if (a_ptr->delay > 0) {
					a_ptr->delay -= 1;
					num_delayed += 1;
				} else {

					activity = dequeue(&(pool[i].request_release));
					type = activity.type;
					if (type == INIT) {
						state[i] = ALIVE;
					} else if (type == REQ) {
						int req = activity.num_resources;
						requested[activity.p_ind][activity.r_ind] += activity.num_resources;

						//Abort if the process breaks its initial claim.
						if (req > max[activity.p_ind][activity.r_ind] ||
							requested[activity.p_ind][activity.r_ind] > max[activity.p_ind][activity.r_ind]) {

							printf("Process %d broke its initial claim and has been aborted.\n", activity.p_ind);	
							state[activity.p_ind] = DEAD;
							term_count += 1;
							pool[activity.p_ind].aborted = 1;
							pool[activity.p_ind].finished_at = cycle;
							for (i = 0; i < resources_sz; i ++) {
								available_next_cycle[i] += used_resources[i][activity.p_ind];
								used_resources[i][activity.p_ind] = 0;
								current[activity.p_ind][i] = 0;
							}
						} else {
							current[activity.p_ind][activity.r_ind] += activity.num_resources;
							resources[activity.r_ind] -= activity.num_resources;

							//Only grant the request if a safe state can be found from the request.
							if (resources[activity.r_ind] < 0 || !safe_state(pool_sz, resources_sz, state, current, resources, max)) {
								current[activity.p_ind][activity.r_ind] -= activity.num_resources;
								resources[activity.r_ind] += activity.num_resources;
								requested[activity.p_ind][activity.r_ind] -= activity.num_resources;
								pool[activity.p_ind].time_waited += 1;

								//Not yet granted, but might be some cycles later on.
								enqueue(&pending_requests, activity);
							} else {
								used_resources[activity.r_ind][activity.p_ind] += activity.num_resources;
								pool[activity.p_ind].delay = activity.delay;
								pool[activity.p_ind].blocked = 0;
							}
						}
					} else if (type == REL) { 			
						
						available_next_cycle[activity.r_ind] += activity.num_resources;
						used_resources[activity.r_ind][i] -= activity.num_resources;

						//Reset number of resources requested since last release.
						requested[activity.p_ind][activity.r_ind] = 0;
						current[activity.p_ind][activity.r_ind] = 0;
						pool[i].delay = activity.delay;
						
					} else if (type == TERM) {
						used_resources[activity.r_ind][i] = 0;
						state[i] = DEAD;
						term_count += 1;
						pool[i].finished_at = cycle;
					} else {
						printf("Invalid activity in input.");
						exit(0);
					}
				}
			} else if (pool[i].blocked == 0) {
				pool[i].delay -= 1;
				num_delayed += 1;
			}
		}			

		cycle ++;
	}

	summarize_results("Banker's", pool, pool_sz);
}

int safe_state(int pool_sz, int resources_sz, int state[], int current[pool_sz][resources_sz], int available[], int max[pool_sz][resources_sz]) {

	int running = 0;
	int blocked;
	int safe;
	int i, j;

	for (i = 0; i < pool_sz; i ++) {
		running += state[i];
	}

	int state_copy[pool_sz];
	memcpy(state_copy, state, sizeof(int) * pool_sz);
	int available_copy[resources_sz];
	memcpy(available_copy, available, sizeof(int) * resources_sz);
	int needs;

	int term_count = 0;

	while (term_count < running) {
		safe = 0; //Pessimism
		//Consider all running processes and find an ordering that guarantees process termination.
		for (i = 0; (i < pool_sz && !safe); i ++) {
			if (state_copy[i]) {
				blocked = 0;
				for (j = 0; (j < resources_sz && !blocked); j ++) {
					needs = max[i][j] - current[i][j];
					blocked = (needs > available_copy[j]) ? 1 : 0;
				}

				//Pretend the process terminates, and ignore all other requests until then. 
				//Release all used resources upon termination.

				safe = !blocked;
				for (j = 0; j < resources_sz && !blocked; j ++) {
					available_copy[j] += current[i][j];
				}

				if (!blocked) {
					state_copy[i] = DEAD;
					term_count ++;
				}
			}
		}

		//No such ordering was found.
		if (!safe) {
			break;
		} 
	}

	return safe;
}

void summarize_results(char header[10], process pool[], int pool_sz) {
	int i;
	printf("\n");
	printf("\t%s\t\n", header);
	int total_waited = 0;
	int turnaround = 0;
	for (i = 0; i < pool_sz; i ++) {
		if (!pool[i].aborted) {
			printf("Task %d%6d%4d   %.0f%%\n", (i + 1), pool[i].finished_at, pool[i].time_waited, (100.0 * pool[i].time_waited) / pool[i].finished_at);
			turnaround += pool[i].finished_at;
			total_waited += pool[i].time_waited;
		} else {
			printf("Task %d\t%s\n", (i + 1), "aborted");
		} 
	}
	printf("total %6d%4d   %.0f%%\n", turnaround, total_waited, (100.0 * total_waited) / turnaround);
	printf("\n");
}