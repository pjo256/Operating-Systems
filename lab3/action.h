#ifndef INCLUDED_ACTION
#define INCLUDED_ACTION

enum action_type { INIT, REQ, REL, TERM };

typedef struct action {

	enum action_type type;
	int delay;
	int r_ind;
	int p_ind;
	int num_resources;
} action;
#endif