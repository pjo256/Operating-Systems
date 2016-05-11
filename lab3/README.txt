
In this directory, you will find a linked list implementation of a queue and the resource manager code, containing the 2 specified resource allocation algorithms.

	i)	Compilation command: gcc manager.c queue.c -o manager
	ii)	Running command (takes at most two command line arguments)
			Unix -> ./manager <input_file_name>
			MSDOS -> manager.exe [--verbose] <input_file_name>

There is one constraint on input: the program assumes that the maximum string length of an activity is 10. If any activities with strlen > 10 are used in this simulation,
you should change this constant appropriately. Furthermore, any unrecognized activity will result in a program exit.