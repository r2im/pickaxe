/*
 * ltl.h
 *
 *  Created on: 14.05.2014
 *      Author: Margus.Räim
 */

#ifndef LTL_H_
#define LTL_H_

#define ltl_not      0
#define ltl_and      1
#define ltl_or       2
#define ltl_imp      3
#define ltl_iff	     4
#define ltl_x        6
#define ltl_f        7
#define ltl_g        8
#define ltl_u        9
#define ltl_const   10
#define ltl_atomic  11
#define ltl_ph      12

struct ltl_expr
{
	char *name;
	struct ltl_expr *expr1;
	struct ltl_expr *expr2;
	int op;

	//in case of placeholders, the index in the order of appearance
	int ph_i;
	char *ph_val;
};

typedef struct ltl_expr LTLExpr;

struct ltl_root
{
	LTLExpr* expr;

	int numphs;
	LTLExpr **phs;
	VarArray **ph_vals;
};

typedef struct ltl_root LTLRoot;

void to_string(LTLExpr *ltl, FILE *fp, int resolve_ph);
LTLExpr *create_atomic(char *atomic);
LTLExpr *create_const(char *c);
LTLExpr *create_placeholder(char *ph);
LTLExpr *create_ltl(LTLExpr *expr1, LTLExpr *expr2, int op);
LTLRoot *create_root(LTLExpr *expr);
void init_placeholders(LTLRoot *root, LTLExpr *ltl);
#endif /* LTL_H_ */
