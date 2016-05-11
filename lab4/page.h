#ifndef INCLUDED_PAGE
#define INCLUDED_PAGE
typedef struct page {
	int owner_pid;
	int frame_ind;
	int page_num;

	int time_loaded;
	int num_evictions;
} page;
#endif