#ifndef INCLUDED_PROCESS
#define INCLUDED_PROCESS
#include "queue.h"
#include "action.h"
typedef struct process {
	int pid;
	int time_waited;
	int finished_at;
	int aborted;
	int blocked;
	int delay;
	int handled;

	queue request_release;
} process;
#endif