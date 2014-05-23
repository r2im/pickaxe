/*
 * ctl.h
 *
 *  Created on: 14.05.2014
 *      Author: Margus.Räim
 */

#ifndef CTL_H_
#define CTL_H_

#define ctl_not      0
#define ctl_and      1
#define ctl_or       2
#define ctl_imp      3
#define ctl_iff	     4
#define ctl_ex       6
#define ctl_ef       7
#define ctl_eg       8
#define ctl_eu       9
#define ctl_ax      10
#define ctl_af      11
#define ctl_ag      12
#define ctl_au      13
#define ctl_const   14
#define ctl_atomic  15
#define ctl_ph      16

#include "pickaxe.h" //some how the build system of eclipse needs this...
struct ctl_expr
{
	char *name;
	struct ctl_expr *expr1;
	struct ctl_expr *expr2;
	int op;

	//in case of placeholders, the index in the order of appearance
	int ph_i;
	char *ph_val;
};

typedef struct ctl_expr CTLExpr;

struct ctl_root
{
	CTLExpr* expr;

	int numphs;
	CTLExpr **phs;
	VarArray **ph_vals;
};

typedef struct ctl_root CTLRoot;

void to_string(CTLExpr *ctl, FILE *fp, int resolve_ph);
CTLExpr *create_atomic(char *atomic);
CTLExpr *create_const(char *c);
CTLExpr *create_placeholder(char *ph);
CTLExpr *create_ctl(CTLExpr *expr1, CTLExpr *expr2, int op);
CTLRoot *create_root(CTLExpr *expr);
void init_placeholders(CTLRoot *root, CTLExpr *ctl);
#endif /* CTL_H_ */
