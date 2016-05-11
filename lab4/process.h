#ifndef INCLUDED_PROCESS
#define INCLUDED_PROCESS
typedef struct process {
	int pid;
	double probA;
	double probB;
	double probC;

	int curr_reference;
	int num_references;

	int residency;
	int evicted;
	int faults;
} process;
#endif