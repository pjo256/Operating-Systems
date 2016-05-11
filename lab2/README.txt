
In this directory, you will find a linked list implementation of a queue and the scheduler code, containing the 4 specified scheduling algorithms.

	i)	Compilation command: gcc scheduler.c queue.c -o scheduler
	ii)	Running command (takes at most two command line arguments)
			Unix -> ./scheduler [--verbose] <input_file_name>
			MSDOS -> scheduler.exe [--verbose] <input_file_name>

Note that process statistics is ordered by occurrence in the original input, while verbose output is ordered by ascending order following the process comparator,
to make it easier to verify the verbose output.