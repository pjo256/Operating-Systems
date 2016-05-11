#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

typedef unsigned int mem_addr_t;


int main(int argc, char **argv)
{
	int num_def_pairs;
	int num_symbols;
	int MAX_ADDRESS = 601;

	char * definitions[MAX_ADDRESS];
	char * use_list[];
	char * symbol;
	mem_addr_t relative_addr;
	mem_addr_t absolute_addr;

	int module_length;
	unsigned int absolute_start = 0;

	/* 1 x 2
	 * 2 x z
	 * 3 R3000 E4000 I5000
	 *
	 *
	*/

	while (scanf("%d", &num_def_pairs) != EOF)
	{
		for (int i = 0; i < num_def_pairs; i++)
		{
			scanf("%s", &symbol);
			scanf("%d", &relative_addr);

			absolute_addr = relative_addr + absolute_start;

			definitions.map[absolute_addr] = (char *) malloc(strlen(symbol) * sizeof(char));
			strcpy(definitions.map[absolute_addr], symbol);
		}

		scanf("%d", &num_symbols);
		use_list = malloc(num_symbols * sizeof(char *));
		for (int i = 0; i < num_symbols; i++)
		{
			scanf("%s", &symbol);
			use_list[i] = (char *) malloc(strlen(symbol) * sizeof(char));
			strcpy(use_list[i], symbol);
		}

		scanf("%d", &module_length);
		link_module(module_length, definitions, use_list, absolute_start);
		absolute_start += module_length;
	}
}

int link_module(int module_length, char * defs[], char * use_list[], int absolute_start)
{
	return 1;
}