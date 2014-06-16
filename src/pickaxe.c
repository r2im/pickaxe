#include <stdio.h>
#include <string.h>
#include <math.h>

#include <bdd.h>
#include <expat.h>

#include "pickaxe.h"
#include "transition_system.h"
#include "ctl.h"

extern int yyparse(void);
extern void init_yy(void);
CTLExpr *parsed;

static char *my_true = "true";

char **current_phs;

static BDD check_eu(TransitionSystem *model, BDD l, BDD r) {

	BDD res = bdd_addref(r);
	BDD tmp = bdd_addref(pre_exists(model, res)); //get predecessors
	BDD tmp2 = bdd_addref(bdd_and(l, tmp)); //filter out states that are not included in l
	BDD tmp3 = bdd_addref(bdd_or(res, tmp2)); //append result
	while (res != tmp3)
	{
		bdd_delref(res);
		res = bdd_addref(tmp3);
		bdd_delref(tmp);
		bdd_delref(tmp2);
		bdd_delref(tmp3);
		tmp = bdd_addref(pre_exists(model, res)); //get predecessors
		tmp2 = bdd_addref(bdd_and(l, tmp)); //filter out states that are not included in l
		tmp3 = bdd_addref(bdd_or(res, tmp2)); //append result
	}
	bdd_delref(tmp);
	bdd_delref(tmp2);
	bdd_delref(tmp3);

	return res;
}

static BDD check_au(TransitionSystem *model, BDD l, BDD r) {

	BDD res = bdd_addref(r);
	BDD tmp = bdd_addref(pre_all(model, res)); //get predecessors
	BDD tmp2 = bdd_addref(bdd_and(l, tmp)); //filter out states that are not included in l
	BDD tmp3 = bdd_addref(bdd_or(res, tmp2)); //append result
	while (res != tmp3)
	{
		bdd_delref(res);
		res = bdd_addref(tmp3);
		bdd_delref(tmp);
		bdd_delref(tmp2);
		bdd_delref(tmp3);
		tmp = bdd_addref(pre_all(model, res)); //get predecessors
		tmp2 = bdd_addref(bdd_and(l, tmp)); //filter out states that are not included in l
		tmp3 = bdd_addref(bdd_or(res, tmp2)); //append result
	}
	bdd_delref(tmp);
	bdd_delref(tmp2);
	bdd_delref(tmp3);

	return res;
}
static BDD check_ctl(TransitionSystem *model, CTLExpr *ctl)
{
	BDD res = 0;
	BDD tmp;
	BDD tmp2;
	Labels *l;
	switch(ctl->op)
	{
	case ctl_not:
		tmp = check_ctl(model, ctl->expr1);
		res = bdd_addref(bdd_apply(model->all_states, tmp, bddop_diff));
		bdd_delref(tmp);
		break;
	case ctl_and:
		tmp = check_ctl(model, ctl->expr1);
		tmp2 = check_ctl(model, ctl->expr2);
		res = bdd_addref(bdd_and(tmp, tmp2));
		bdd_delref(tmp);
		bdd_delref(tmp2);
		break;
	case ctl_or:
		tmp = check_ctl(model, ctl->expr1);
		tmp2 = check_ctl(model, ctl->expr2);
		res = bdd_addref(bdd_or(tmp, tmp2));
		bdd_delref(tmp);
		bdd_delref(tmp2);
		break;
	case ctl_imp:
	{
		//a -> b is !a | b
		CTLExpr anot; anot.op = ctl_not; anot.expr1 = ctl->expr1;
		CTLExpr or; or.op = ctl_or; or.expr1 = &anot; or.expr2 = ctl->expr2;
		res = check_ctl(model, &or);
		break;
	}
	case ctl_iff:
	{
		// a <-> b is (!a | b) & (a | !b)
		CTLExpr anot2 = {.op = ctl_not, .expr1 = ctl->expr1};
		CTLExpr bnot; bnot.op = ctl_not; bnot.expr1 = ctl->expr2;
		CTLExpr or1; or1.op = ctl_or; or1.expr1 = &anot2; or1.expr2 = ctl->expr2;
		CTLExpr or2; or2.op = ctl_or; or2.expr1 = &bnot; or2.expr2 = ctl->expr1;
		CTLExpr and; and.op = ctl_and; and.expr1 = &or1, and.expr2 = &or2;
		res = check_ctl(model, &and);
		break;
	}
	case ctl_ex:
		tmp = check_ctl(model, ctl->expr1);
		res = bdd_addref(pre_exists(model, tmp));
		bdd_delref(tmp);
		break;
	case ctl_ef:
		//check inner expression
		res = check_ctl(model, ctl->expr1);
		//get predecessors
		tmp = bdd_addref(pre_exists(model, res));
		tmp2 = bdd_addref(bdd_or(res, tmp));
		while (res != tmp2) //while there is no predecessors
		{
			bdd_delref(res);
			res = bdd_addref(tmp2);
			bdd_delref(tmp);
			bdd_delref(tmp2);
			tmp = bdd_addref(pre_exists(model, res));
			tmp2 = bdd_addref(bdd_or(res, tmp));
		}
		bdd_delref(tmp);
		bdd_delref(tmp2);

		break;
	case ctl_eg:
		res = check_ctl(model, ctl->expr1);
		tmp = bdd_addref(model->all_states);
		while (res != tmp) {
			bdd_delref(tmp);
			tmp = bdd_addref(res);
			bdd_delref(res);
			tmp2 = bdd_addref(pre_exists(model, tmp));
			res = bdd_addref(bdd_and(tmp, tmp2));
			bdd_delref(tmp2);
		}
		bdd_delref(tmp);
		break;
	case ctl_eu:
		tmp = check_ctl(model, ctl->expr1);
		tmp2 = check_ctl(model, ctl->expr2);
		res = check_eu(model, tmp, tmp2);
		bdd_delref(tmp);
		bdd_delref(tmp2);
		break;
	case ctl_ax:
		tmp = check_ctl(model, ctl->expr1);
		res = bdd_addref(pre_exists(model, tmp));
		bdd_delref(tmp);
		break;
	case ctl_af:
		//check inner expression
		res = check_ctl(model, ctl->expr1);
		//get predecessors
		tmp = bdd_addref(pre_all(model, res));
		tmp2 = bdd_addref(bdd_or(res, tmp));
		while (res != tmp2) //while there is no predecessors
		{
			bdd_delref(res);
			res = bdd_addref(tmp2);
			bdd_delref(tmp);
			bdd_delref(tmp2);
			tmp = bdd_addref(pre_all(model, res));
			tmp2 = bdd_addref(bdd_or(res, tmp));
		}
		bdd_delref(tmp);
		bdd_delref(tmp2);
		break;
	case ctl_ag:
		res = check_ctl(model, ctl->expr1);
		tmp = bdd_addref(model->all_states);
		while (res != tmp) {
			bdd_delref(tmp);
			tmp = bdd_addref(res);
			bdd_delref(res);
			tmp2 = bdd_addref(pre_all(model, tmp));
			res = bdd_addref(bdd_and(tmp, tmp2));
			bdd_delref(tmp2);
		}
		bdd_delref(tmp);
		break;
	case ctl_au:
		tmp = check_ctl(model, ctl->expr1);
		tmp2 = check_ctl(model, ctl->expr2);
		res = check_au(model, tmp, tmp2);
		bdd_delref(tmp);
		bdd_delref(tmp2);
		break;
	case ctl_const:
		if (strcmp(ctl->name, my_true) == 0)
			res = bdd_addref(model->all_states);
		else
			res = bdd_addref(0);
		printf("constant: %s\n", ctl->name);
		break;
	case ctl_atomic:
	{
		l = get_labels_for(model, ctl->name);  //can create new labels if not exists
		if (l->states_size > 0)
			res = bdd_addref(l->states_bdd);
		else
			res = bdd_addref(0);
		break;
	}
	case ctl_ph:
	{
		CTLExpr *atomic = create_atomic(current_phs[ctl->ph_i]);
		ctl->ph_val = current_phs[ctl->ph_i]; //needed only for printing
		res = check_ctl(model, atomic);
		free(atomic);
		break;
	}
	default:
		printf("ERROR: unknown operator");
	}

	return res;
}

void check_query(TransitionSystem *model, CTLRoot *ctl)
{

	int numphs = ctl->numphs;
	if (numphs == 0)
	{
		//TODO: remove duplicate code
		BDD res = check_ctl(model, ctl->expr);
		double s = support(model, res);
		bdd_delref(res);
		printf("%.2f\t", s);
		to_string(ctl->expr, stdout, 1);
		printf("\n");
		return;
	}

	char *cphs[numphs];
	current_phs = cphs;
	int ph_indexes[numphs];
	for (int i = 0; i < numphs; i++) ph_indexes[i] = 0; //set all indexes to 0

	//calculate the number of loops
	int numloops = 1;
	for (int i = 0; i < numphs; i++)
	{
		if (NULL == ctl->ph_vals[i])
		{
			ctl->ph_vals[i] = new_array();
			for (int j = 0; j < model->activities_size; j++)
			{
				add_element(ctl->ph_vals[i], model->labels[j]->activity);
			}
		}
		numloops *= ctl->ph_vals[i]->length;
	}


	for (int i = 0; i < numloops; i++)
	{

		//make assignments
		for (int n = 0; n < numphs; n++)
		{
			int li = ph_indexes[n];
			VarArray *a = ctl->ph_vals[n];
			current_phs[n] = a->elements[li];
//			printf("current_phs[%d] = %s\n", n, a->elements[li]);
			//increment index
			li = (li + 1) % ctl->ph_vals[n]->length;
			ph_indexes[n] = li;

			//when index makes a full round, increment next. otherwise no need to increment other(s)
			if (li != 0 && i != 0) {
				break;
			}
		}

		//check for duplicate assignments
		for (int n = 0; n < numphs - 1; n++)
			for (int m = n + 1; m < numphs; m++)
				if (ph_indexes[n] == ph_indexes[m])
					goto end;

		BDD res = check_ctl(model, ctl->expr);
		double s = support(model, res);
		bdd_delref(res);
		printf("%.2f\t", s);
		to_string(ctl->expr, stdout, 1);
		printf("\n");

		end:
		continue;
	}
}


static int event_i = 0;
static char *trace[500];
static char *last_event;
static char *last_lc = NULL;
static char event[200];

static TransitionSystem *model;

static void start_element(void *user_data, const char *name, const char **atts)
{
  if (strcmp("trace", name) == 0)
  {
	  event_i = 0;
  }
  else if (strcmp("event", name) == 0)
  {
	  //printf("event start\n");
  }
  else if (strcmp("string", name) == 0)
  {
	  if(strcmp("concept:name", atts[1]) == 0)
		  last_event = strdup(atts[3]);
	  else if (strcmp("lifecycle:transition", atts[1]) == 0)
	  {
		  last_lc = strdup(atts[3]);
	  }
  }
}

static void end_element(void *user_data, const char *name)
{
  if (strcmp("trace", name) == 0)
  {
	  trace[event_i] = NULL;
//	  printf("adding trace %d\n", event_i);
	  add_trace(model, trace);
//	  printf("added\n");
  }
  else if (strcmp("event", name) == 0)
  {

	  strcpy(event, last_event);
	  if (last_lc != NULL)
	  {
		  strcat(event, "#");
		  strcat(event, last_lc);
		  free(last_lc);
		  last_lc = NULL;
	  }
	  free(last_event);
	  trace[event_i] = strdup(event);
	  event_i++;
  }
}


static void parse_log(const char *log_file)
{
	FILE *xes = fopen(log_file, "r");
	if (!xes)
	{
		printf("File not found: %s\n", log_file);
		exit(1);
	}
	char buf[BUFSIZ];
	XML_Parser parser = XML_ParserCreate(NULL);
	int done;
	XML_SetElementHandler(parser, start_element, end_element);
	do {
		int len = (int)fread(buf, 1, sizeof(buf), xes);
		done = len < sizeof(buf);
		if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
			fprintf(stderr,
			"%s at line %lu\n",
			XML_ErrorString(XML_GetErrorCode(parser)),
			XML_GetCurrentLineNumber(parser));
			return;
		}
	} while (!done);
	XML_ParserFree(parser);
}

#include <time.h>
#include <sys/time.h>

//#define _DEBUG 1
int main(int argc, char **args)
{

	struct timeval start, end;
	long delta = 0;
	model = create_emtpty_model();
#ifndef _DEBUG
	char * log_file;
	if (argc < 2)
	{
		printf("No log file name given, using ./log.xes\n");
		log_file = "D:\\test_logs\\var_max_trace_size\\a_test_traces_100_alphabet_10_maxeventstraces_5.xes";
	}
	else
	{
		log_file = args[1];
	}
	parse_log(log_file);
#else

	char *trace1[] = {"A", "A", NULL};
	char *trace2[] = {"D", "A", "B", "A", NULL};
	char *trace3[] = {"A", "B", "B", "D", NULL};
	char *trace4[] = {"G", "F", NULL};
	char *trace5[] = {"A", "A", "B", NULL};

	add_trace(model, trace1);
	add_trace(model, trace2);
//	add_trace(model, trace3);
//	add_trace(model, trace4);
//	add_trace(model, trace5);



	create_bdds(model);
	BDD pres = bdd_apply(model->all_states, model->states_bdds[2]));
//	BDD expected = bdd_or(model->states_bdds[1], model->states_bdds[4]);
//	printf("%d", pres);
//	print_model(model);
//	parsed = create_ctl(create_atomic("B"), NULL, ctl_not);
//	CTLRoot *d_ctl = create_root(parsed);
//	init_placeholders(d_ctl, parsed);
//	VarArray *x_vals = new_array();
//	add_element(x_vals, "A");
//	add_element(x_vals, "B");

//	ctl->ph_vals[0] = x_vals;

//	check_query(model, d_ctl);
//	bdd_gbc();
//	printf("%d", model->states_size);
//	print_model(model);

	return 0;
#endif
	create_bdds(model);
//	printf("initial nodes: %d\n", bdd_getnodenum());

//	printf("%d states in the transition system.\n", model->states_size);
//	print_model(model);
/***************FILTER*********************/
	VarArray *x_vals0 = new_array();
	add_element(x_vals0, "A_SUBMITTED#complete");
	add_element(x_vals0, "W_Completeren aanvraag#schedule");
	add_element(x_vals0, "A_DECLINED#complete");
	add_element(x_vals0, "O_DECLINED#complete");
	add_element(x_vals0, "A_FINALIZED#complete");
	add_element(x_vals0, "O_SENT#complete");
	add_element(x_vals0, "O_DECLINED#complete");

	VarArray *x_vals1 = new_array();
	add_element(x_vals1, "A_SUBMITTED#complete");
	add_element(x_vals1, "W_Completeren aanvraag#schedule");
	add_element(x_vals1, "A_DECLINED#complete");
	add_element(x_vals1, "O_DECLINED#complete");
	add_element(x_vals1, "A_FINALIZED#complete");
	add_element(x_vals1, "O_SENT#complete");
	add_element(x_vals1, "O_DECLINED#complete");

	VarArray *x_vals2 = new_array();
	add_element(x_vals2, "A_SUBMITTED#complete");
	add_element(x_vals2, "W_Completeren aanvraag#schedule");
	add_element(x_vals2, "A_DECLINED#complete");
	add_element(x_vals2, "O_DECLINED#complete");
	add_element(x_vals2, "A_FINALIZED#complete");
	add_element(x_vals2, "O_SENT#complete");
	add_element(x_vals2, "O_DECLINED#complete");

/***************FILTER*********************/
//	init_yy();
//	int i = 1;
//	while(i != 0)
//	{
//		i = yyparse();
//		if (i == 0 || parsed == NULL)
//			break;

//		to_string(parsed, stdout, 0);
parsed = create_ctl(create_ctl(create_placeholder("?x"), create_ctl(create_atomic("A_ACTIVATED#complete"), NULL, ctl_ef), ctl_imp), NULL, ctl_eg);
//to_string(parsed, stdout, 0);
//return 0;
		CTLRoot *ctl = create_root(parsed);
//		ctl->ph_vals[0] = x_vals0;
//		ctl->ph_vals[1] = x_vals1;
//		ctl->ph_vals[2] = x_vals2;
		init_placeholders(ctl, parsed);
		gettimeofday(&start, NULL);
		check_query(model, ctl);
		gettimeofday(&end, NULL);
		delta += (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
//		printf("%s\t", log_file);
//		to_string(parsed, stdout, 0);
//		printf("\t%ld\n", delta);
//	}
#ifndef _DEBUG
	printf("%s\t%ld\t%d\n", log_file, delta, model->states_size);
#endif

//	model = create_emtpty_model();
//	char *trace1[] = {"A", "B", "C", "D", NULL};
//	char *trace2[] = {"A", "A", "B", "A", "D", NULL};
//	char *trace3[] = {"A", "B", "C", "D", NULL};
//
//	add_trace(model, trace1);
//	add_trace(model, trace2);
//	add_trace(model, trace3);
//
//	create_bdds(model);
//	print_model(model);
//
//	CTLExpr* ctl = parsed; //create_placeholder("?x");
//	init_placeholders(ctl, ctl);
//	check_query(model, ctl);


//TESTING
//	Labels *l = get_labels_for(model, "B");

//	BDD res = check_ctl(model, &chain_succ);
//	double s = support(model, res);
//	printf("%.2f", s);
//	bdd_printtable(res);
//
////	print_model(model);
////
//	BDD expected = bdd_or(model->states_bdds[0], model->states_bdds[1]);
//	expected = bdd_or(expected, model->states_bdds[2]);
//	expected = bdd_or(expected, model->states_bdds[3]);
//	expected = bdd_or(expected, model->states_bdds[4]);
//	expected = bdd_or(expected, model->states_bdds[5]);
//	expected = bdd_or(expected, model->states_bdds[6]);
//	expected = bdd_or(expected, model->states_bdds[7]);
////	expected = bdd_or(expected, model->states_bdds[8]);
//
//	if (res == expected)
//		printf("allright");
//	else
//		printf("oi");
//	char ch = getchar();
	return 0;
}
