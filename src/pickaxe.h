/*
 * pickaxe.h
 *
 *  Created on: 23.05.2014
 *      Author: Margus.Räim
 */

#ifndef PICKAXE_H_
#define PICKAXE_H_

struct var_array
{
	size_t length;
	void **elements;
};

typedef struct var_array VarArray;

VarArray *new_array();
void add_element(VarArray *a, void *element);
void del_array(VarArray *a);

#endif /* PICKAXE_H_ */
