#include <stdlib.h>
#include <stdio.h>

#include "pickaxe.h"

void add_element(VarArray *a, void *element)
{

	a->length++;
	a->elements = realloc(a->elements, a->length);
	if (NULL == a->elements)
	{
		printf("core: add_elememnt realloc failed");
		exit(-1);
	}
	a->elements[a->length - 1] = element;
}

VarArray *new_array()
{
	VarArray *a = malloc(sizeof(VarArray *));
	a->length = 0;
	a->elements = NULL;

	return a;
}

void del_array(VarArray *a)
{
	free(a);
}
