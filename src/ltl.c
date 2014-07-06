#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pickaxe.h"
#include "ltl.h"

void to_string(LTLExpr *ltl, FILE *fp, int resolve_ph)
{
	switch(ltl->op)
	{
	case ltl_not:
		fprintf(fp, "!(");
		to_string(ltl->expr1, fp, resolve_ph);
		fprintf(fp, ")");
		break;
	case ltl_and:
//		fprintf(fp, "(");
		to_string(ltl->expr1, fp, resolve_ph);
		fprintf(fp, " & ");
		to_string(ltl->expr2, fp, resolve_ph);
//		fprintf(fp, ")");
		break;
	case ltl_or:
//		fprintf(fp, "(");
		to_string(ltl->expr1, fp, resolve_ph);
		fprintf(fp, " | ");
		to_string(ltl->expr2, fp, resolve_ph);
//		fprintf(fp, ")");
		break;
	case ltl_imp:
//		fprintf(fp, "(");
		to_string(ltl->expr1, fp, resolve_ph);
		fprintf(fp, " -> ");
		to_string(ltl->expr2, fp, resolve_ph);
//		fprintf(fp, ")");
		break;
	case ltl_iff:
//		fprintf(fp, "(");
		to_string(ltl->expr1, fp, resolve_ph);
		fprintf(fp, " <-> ");
		to_string(ltl->expr2, fp, resolve_ph);
//		fprintf(fp, ")");
		break;
	case ltl_x:
		fprintf(fp, "X(");
		to_string(ltl->expr1, fp, resolve_ph);
		fprintf(fp, ")");
		break;
	case ltl_f:
		fprintf(fp, "F(");
		to_string(ltl->expr1, fp, resolve_ph);
		fprintf(fp, ")");
		break;
	case ltl_g:
		fprintf(fp, "G(");
		to_string(ltl->expr1, fp, resolve_ph);
		fprintf(fp, ")");
		break;
	case ltl_u:
		fprintf(fp, "[");
		to_string(ltl->expr1, fp, resolve_ph);
		fprintf(fp, " U ");
		to_string(ltl->expr2, fp, resolve_ph);
		fprintf(fp, "]");
		break;
	case ltl_const:
	case ltl_atomic:
		fprintf(fp, "%s", ltl->name);
		break;
	case ltl_ph:
		if(resolve_ph)
			fprintf(fp, "%s", ltl->ph_val);
		else
			fprintf(fp, "%s", ltl->name);

		break;
	default:
		printf("ERROR: unknown ltl expression");
	}
}

LTLExpr *create_atomic(char *atomic)
{
	LTLExpr *ltl = malloc(sizeof(LTLExpr));
	ltl->name = strdup(atomic);
	ltl->op = ltl_atomic;

	return ltl;
}

LTLExpr *create_const(char *c)
{
	LTLExpr *ltl = malloc(sizeof(LTLExpr));
	ltl->name = strdup(c);
	ltl->op = ltl_const;

	return ltl;
}

LTLExpr *create_placeholder(char *ph)
{
	LTLExpr *ltl = malloc(sizeof(LTLExpr));
	ltl->name = strdup(ph);
	ltl->op = ltl_ph;
	ltl->ph_i = 0;
	ltl->ph_val = NULL;

	return ltl;
}

LTLExpr *create_ltl(LTLExpr* expr1, LTLExpr* expr2, int op)
{
	LTLExpr *ltl = malloc(sizeof(LTLExpr));
	ltl->expr1 = expr1;
	ltl->expr2 = expr2;
	ltl->op = op;

	return ltl;
}

LTLRoot *create_root(LTLExpr* expr)
{
	LTLRoot *root = malloc(sizeof(LTLRoot));
	root->expr = expr;
	root->numphs = 0;
	root->phs = NULL;
	root->ph_vals = malloc(5 * sizeof(VarArray *)); //TODO: remove magic number
	if (NULL == root->ph_vals)
	{
		fprintf(stderr, "ltl.c in create_root: malloc failed\n");
		exit(-1);
	}
	for (int i = 0; i < 5; i++) root->ph_vals[i] = NULL;

	return root;
}

void init_placeholders(LTLRoot* root, LTLExpr* ltl)
{

	if (ltl->op == ltl_atomic || ltl->op == ltl_const)
		return; // in leaf node

	if (ltl->op == ltl_ph)
	{
		if (root->numphs == 0)
		{
			//first placeholder
			root->numphs = 1;
			root->phs = malloc(sizeof(LTLExpr *));
			root->phs[0] = ltl;
			ltl->ph_i = 0;
		}
		else
		{
			int found = 0;
			//find if similar placeholders exist
			for (int i = 0; i < root->numphs; i++)
			{
				if (strcmp(ltl->name, (root->phs[i])->name) == 0)
				{
					found = 1;
					ltl->ph_i = (root->phs[i])->ph_i;
					break;
				}
			}

			//not found, add new one
			if (found == 0)
			{
				ltl->ph_i = root->numphs;
				root->numphs++;
				root->phs = realloc(root->phs, sizeof(LTLExpr *) * root->numphs);
				root->phs[ltl->ph_i] = ltl;
			}
		}

		return;
	}

	init_placeholders(root, ltl->expr1);

	if (ltl->expr2)
		init_placeholders(root, ltl->expr2);
}

