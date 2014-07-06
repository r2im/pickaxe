#include <stdlib.h>
#include <stdio.h>

#include "pickaxe.h"

void add_element(VarArray *a, char *element)
{
	a->length++;
	a->elements = realloc(a->elements, (a->length) * sizeof(char*));
	if (NULL == a->elements)
	{
		fprintf(stderr, "core: add_elememnt realloc failed\n");
		exit(-1);
	}
	a->elements[a->length - 1] = element;
}

VarArray *new_array()
{
	VarArray *a = malloc(sizeof(VarArray));
	if (NULL == a)
	{
		fprintf(stderr, "core: new_array malloc failed\n");
		exit(-1);
	}
	a->length = 0;
	a->elements = NULL;

	return a;
}

void del_array(VarArray *a)
{
	free(a);
}
