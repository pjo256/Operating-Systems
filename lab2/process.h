#ifndef INCLUDED_PROCESS
#define INCLUDED_PROCESS
#define SHORTEST 0
#define EARLIEST 1
typedef struct process {
	int pid;
	int sorted_ind;
	int arrival_time;
	int cpu_time;
	int cpu_time_left;
	int io_burst;
	int io_burst_gen;
	int cpu_burst_gen;
	int cpu_burst;
	int started_wait;
	int time_blocked;
	int time_waited;
	int finished_at;
	int turnaround;
	int cpu_burst_remainder;
} process;
#endif