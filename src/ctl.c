#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pickaxe.h"
#include "ctl.h"

void ctl_to_string(CTLExpr *ctl, FILE *fp, int resolve_ph)
{
	switch(ctl->op)
	{
	case ctl_not:
		fprintf(fp, "!(");
		to_string(ctl->expr1, fp, resolve_ph);
		fprintf(fp, ")");
		break;
	case ctl_and:
//		fprintf(fp, "(");
		to_string(ctl->expr1, fp, resolve_ph);
		fprintf(fp, " & ");
		to_string(ctl->expr2, fp, resolve_ph);
//		fprintf(fp, ")");
		break;
	case ctl_or:
//		fprintf(fp, "(");
		to_string(ctl->expr1, fp, resolve_ph);
		fprintf(fp, " | ");
		to_string(ctl->expr2, fp, resolve_ph);
//		fprintf(fp, ")");
		break;
	case ctl_imp:
//		fprintf(fp, "(");
		to_string(ctl->expr1, fp, resolve_ph);
		fprintf(fp, " -> ");
		to_string(ctl->expr2, fp, resolve_ph);
//		fprintf(fp, ")");
		break;
	case ctl_iff:
//		fprintf(fp, "(");
		to_string(ctl->expr1, fp, resolve_ph);
		fprintf(fp, " <-> ");
		to_string(ctl->expr2, fp, resolve_ph);
//		fprintf(fp, ")");
		break;
	case ctl_ex:
		fprintf(fp, "X(");
		to_string(ctl->expr1, fp, resolve_ph);
		fprintf(fp, ")");
		break;
	case ctl_ef:
		fprintf(fp, "F(");
		to_string(ctl->expr1, fp, resolve_ph);
		fprintf(fp, ")");
		break;
	case ctl_eg:
		fprintf(fp, "G(");
		to_string(ctl->expr1, fp, resolve_ph);
		fprintf(fp, ")");
		break;
	case ctl_eu:
		fprintf(fp, "[");
		to_string(ctl->expr1, fp, resolve_ph);
		fprintf(fp, " U ");
		to_string(ctl->expr2, fp, resolve_ph);
		fprintf(fp, "]");
		break;
	case ctl_ax:
		fprintf(fp, "AX(");
		to_string(ctl->expr1, fp, resolve_ph);
		fprintf(fp, ")");
		break;
	case ctl_af:
		fprintf(fp, "AF(");
		to_string(ctl->expr1, fp, resolve_ph);
		fprintf(fp, ")");
		break;
	case ctl_ag:
		fprintf(fp, "AG(");
		to_string(ctl->expr1, fp, resolve_ph);
		fprintf(fp, ")");
		break;
	case ctl_au:
		fprintf(fp, "A[");
		to_string(ctl->expr1, fp, resolve_ph);
		fprintf(fp, " U ");
		to_string(ctl->expr2, fp, resolve_ph);
		fprintf(fp, "]");
		break;
	case ctl_const:
	case ctl_atomic:
		fprintf(fp, "%s", ctl->name);
		break;
	case ctl_ph:
		if(resolve_ph)
			fprintf(fp, "%s", ctl->ph_val);
		else
			fprintf(fp, "%s", ctl->name);

		break;
	default:
		printf("ERROR: unknown ctl expression");
	}
}

CTLExpr *ctl_create_atomic(char *atomic)
{
	CTLExpr *ctl = malloc(sizeof(CTLExpr));
	ctl->name = strdup(atomic);
	ctl->op = ctl_atomic;

	return ctl;
}

CTLExpr *ctl_create_const(char *c)
{
	CTLExpr *ctl = malloc(sizeof(CTLExpr));
	ctl->name = strdup(c);
	ctl->op = ctl_const;

	return ctl;
}

CTLExpr *ctl_create_placeholder(char *ph)
{
	CTLExpr *ctl = malloc(sizeof(CTLExpr));
	ctl->name = strdup(ph);
	ctl->op = ctl_ph;
	ctl->ph_i = 0;
	ctl->ph_val = NULL;

	return ctl;
}

CTLExpr *ctl_create_ctl(CTLExpr* expr1, CTLExpr* expr2, int op)
{
	CTLExpr *ctl = malloc(sizeof(CTLExpr));
	ctl->expr1 = expr1;
	ctl->expr2 = expr2;
	ctl->op = op;

	return ctl;
}

CTLRoot *ctl_create_root(CTLExpr* expr)
{
	CTLRoot *root = malloc(sizeof(CTLRoot));
	root->expr = expr;
	root->numphs = 0;
	root->phs = NULL;
	root->ph_vals = malloc(5 * sizeof(VarArray *)); //TODO: remove magic number
	if (NULL == root->ph_vals)
	{
		fprintf(stderr, "ctl.c in create_root: malloc failed\n");
		exit(-1);
	}
	for (int i = 0; i < 5; i++) root->ph_vals[i] = NULL;

	return root;
}

void ctl_init_placeholders(CTLRoot* root, CTLExpr* ctl)
{

	if (ctl->op == ctl_atomic || ctl->op == ctl_const)
		return; // in leaf node

	if (ctl->op == ctl_ph)
	{
		if (root->numphs == 0)
		{
			//first placeholder
			root->numphs = 1;
			root->phs = malloc(sizeof(CTLExpr *));
			root->phs[0] = ctl;
			ctl->ph_i = 0;
		}
		else
		{
			int found = 0;
			//find if similar placeholders exist
			for (int i = 0; i < root->numphs; i++)
			{
				if (strcmp(ctl->name, (root->phs[i])->name) == 0)
				{
					found = 1;
					ctl->ph_i = (root->phs[i])->ph_i;
					break;
				}
			}

			//not found, add new one
			if (found == 0)
			{
				ctl->ph_i = root->numphs;
				root->numphs++;
				root->phs = realloc(root->phs, sizeof(CTLExpr *) * root->numphs);
				root->phs[ctl->ph_i] = ctl;
			}
		}

		return;
	}

	init_placeholders(root, ctl->expr1);

	if (ctl->expr2)
		init_placeholders(root, ctl->expr2);
}

