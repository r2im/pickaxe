#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bdd.h>
#include "transition_system.h"

State PSEUDO_END = {.id = 0, .activity = "__END__", .children = NULL, .children_size = 0};

static int initial_numstates = 2;

static int ceil_of_log2_of(int num)
{
	int res = 1;
	int tmp = 2;
	while(tmp < num) {
		tmp *= 2;
		res++;
	}

	return res;
}

/*
 * removes reference from l
 */
static BDD f_bdd_and_with(BDD l, BDD r)
{
	BDD res = bdd_addref(bdd_apply(l, r, bddop_and));
	bdd_delref(l);

	return res;
}

/*
 * removes reference from l
 */
static BDD f_bdd_or_with(BDD l, BDD r)
{
	//deletes left argument
	BDD res = bdd_addref(bdd_apply(l, r, bddop_or));
	bdd_delref(l);

	return res;
}

static void bdd_errhandler(int errcode)
{

	char *errstr = NULL;
	switch (errcode) {
	case BDD_MEMORY:   /* Out of memory */
		errstr = "Out of memory";
		break;
	case BDD_VAR:      /* Unknown variable */
		errstr = "BDD_VAR";
		break;
	case BDD_RANGE:    /* Variable value out of range (not in domain) */
		errstr = "BDD_RANGE";
		break;
	case BDD_DEREF:    /* Removing external reference to unknown node */
		errstr = "BDD_DEREF";
		break;
	case BDD_RUNNING:  /* Called bdd_init() twice whithout bdd_done() */
		errstr = "BDD_RUNNING";
		break;
	case BDD_ORDER:    /* Vars. not in order for vector based functions */
		errstr = "BDD_ORDER";
		break;
	case BDD_BREAK:    /* User called break */
		errstr = "BDD_BREAK";
		break;
	case BDD_VARNUM:  /* Different number of vars. for vector pair */
		errstr = "BDD_VARNUM";
		break;
	case BDD_OP:      /* Unknown operator */
		errstr = "BDD_OP";
		break;
	case BDD_VARSET:  /* Illegal variable set */
		errstr = "BDD_VARSET";
		break;
	case BDD_VARBLK:  /* Bad variable block operation */
		errstr = "BDD_VARBLK";
		break;
	case BDD_DECVNUM: /* Trying to decrease the number of variables */
		errstr = "BDD_DECVNUM";
		break;
	case BDD_REPLACE: /* Replacing to already existing variables */
		errstr = "BDD_REPLACE";
		break;
	case BDD_NODENUM: /* Number of nodes reached user defined maximum */
		errstr = "BDD_NODENUM";
		break;
	case BVEC_SIZE:    /* Mismatch in bitvector size */
		errstr = "BVEC_SIZE";
		break;
	case BVEC_DIVZERO: /* Division by zero */
		errstr = "BVEC_DIVZERO";
		break;
	case BDD_FILE:     /* Some file operation failed */
		errstr = "BDD_FILE";
		break;
	case BDD_FORMAT:   /* Incorrect file format */
		errstr = "BDD_FORMAT";
		break;
	case BDD_NODES:   /* Tried to set max. number of nodes to be fewer */
		errstr = "BDD_NODES";
		break;
	/* than there already has been allocated */
	case BDD_ILLBDD:  /* Illegal bdd argument */
		errstr = "BDD_ILLBDD";
		break;
	case BDD_SIZE:    /* Illegal size argument */
		errstr = "BDD_SIZE";
		break;
	case BVEC_SHIFT:   /* Illegal shift-left/right parameter */
		errstr = "BVEC_SHIFT";
		break;
	default:
		errstr = "UNKNOWN";
		break;
	}
	  printf("BDD error: %s\n", errstr);
	  exit(1);
}

static void init_buddy(int states)
{
	//TODO: calculate the parameters to buddy from the number of states
	bdd_init(70000000, 100000);
	bdd_setmaxincrease(500000);
	bdd_setcacheratio(5);
	bdd_error_hook(bdd_errhandler);
//	bdd_resize_hook(bdd_resizehandler);
//	bdd_gbc_hook(bdd_gbchandler);
//	bdd_reorder_hook(bdd_reorderhandler);
}

static void add_child(State *state, State *child)
{
	state->children_size++;
	state->children = realloc(state->children, sizeof(State *) * (state->children_size));
	state->children[state->children_size - 1] = child;
}

static void add_parent(State *state, State *parent)
{
	state->parents_size++;
	state->parents = realloc(state->parents, sizeof(State *) * (state->parents_size));
	state->parents[state->parents_size - 1] = parent;
}

static void get_transitions(TransitionSystem *model, State *src, State *dest)
{
	if (NULL != src && src->id != 0) //dont make transitions from the pseudo initial state
	{

		Transition *t = malloc(sizeof(Transition));
		if(!t)
		{
			fprintf(stderr, "malloc failed!\n");
			exit(1);
		}
		t->src = src->id;
		if (dest == &PSEUDO_END) //replace transition to pseudo end with transition to itself
			t->dest = src->id;
		else
			t->dest = dest->id;

		model->transition_size++;
		model->transitions = realloc(model->transitions, sizeof(Transition *) * model->transition_size);
		if(!model->transitions)
		{
			fprintf(stderr, "malloc failed!\n");
			exit(1);
		}
		model->transitions[model->transition_size - 1] = t;

	}

	for (int i = 0; i < dest->children_size; i++)
	{
		if (dest->children[i] == NULL)
			continue;
		get_transitions(model, dest, dest->children[i]);
	}
}

//this function is not used, because of the side effect when calculating support
static void merge_similar_children(State *state)
{
	if (state == NULL || state == &PSEUDO_END)
		return;

	int cs = state->children_size;
	State **children = state->children;

	State *child;
	State *child2;
	while(cs > 1)
	{
		//pop last child
		cs--;
		child = children[cs];

		if (child == NULL || child->is_end == 1) //removed or the child is and end state
			continue;

		for(int i = 0; i < cs; i++)
		{
			child2 = children[i];
			if (child2 == NULL || child2->is_end == 1 || child2 == &PSEUDO_END) //removed or the child is and end state
				continue;

			if (strcmp(child->activity, child2->activity) == 0)
			{
				//add all children of child2 to child and remove child2
				for (int j = 0; j < child2->children_size; j++) {
					State* child2_child = child2->children[j];
					if (child2_child == NULL)
						continue;

					int exists = 0;
					for (int m = 0; m < child->children_size; m++)
					{
						//check if exists
						if (child->children[m] == child2_child)
						{
							exists = 1;
							break;
						}
					}
					if (exists == 0)
					{
						add_child(child, child2_child);
					}
					//TODO: for concistency the parent references should also be updated, but not today...
					//remove from parent
				}

				//remove child2
				children[i] = NULL;
				child->cardinality += child2->cardinality;
				//TODO: free some memory
			}
		}
	}

	for (int i = 0; i < state->children_size; i++)
	{
		merge_similar_children(state->children[i]);
	}
}

static void merge_similar_parents(State *state)
{
	if (state == NULL)
		return;

	int ps = state->parents_size;
	if (ps == 0)
		//only initial state has no parents
		return;


	State **parents = state->parents;

	State *parent;
	State *parent2;
	while(ps > 1)
	{
		//pop last parent
		ps--;
		parent = parents[ps];

		if (parent == NULL) //removed
			continue;

		for(int i = 0; i < ps; i++)
		{
			parent2 = parents[i];
			if (parent2 == NULL) //removed
				continue;

			if (strcmp(parent->activity, parent2->activity) == 0)
			{
				//replace parent2 references to parent from all of parent2 parents
				for (int j = 0; j < parent2->parents_size; j++) {
					State* parent2_parent = parent2->parents[j];
					if (parent2_parent == NULL)
						continue;

					//remove from parent
					for (int k = 0; k < parent2_parent->children_size; k++)
					{
						State *pp_child = parent2_parent->children[k];
						if (pp_child == NULL)
							continue;
						if (pp_child == parent2)
						{
							if (parent2_parent->parents_size == 0) //initial node
								parent2_parent->children[k] = NULL; //just delete the reference
							else
								parent2_parent->children[k] = parent; //replace with parent
						}

					}
					if (parent2_parent->parents_size != 0)
					{
						add_parent(parent, parent2_parent);
					}
				}

				//remove parent2
				parents[i] = NULL;
				parent->cardinality++;
				//TODO: remove state (parent2), when there are no children
			}
		}
	}

	for (int i = 0; i < state->parents_size; i++)
	{
		merge_similar_parents(state->parents[i]);
	}
}

Labels* get_labels_for(TransitionSystem *model, char *activity)
{
	Labels *l = NULL;

	for (int i = 0; i < model->activities_size; i++)
	{
		Labels *tmp = model->labels[i];
		if (strcmp(tmp->activity, activity) == 0)
		{
			l = tmp;
			break;
		}
	}

	if (!l)
	{
		//add new bucket for a label
		model->activities_size++;
		model->labels = realloc(model->labels, sizeof(Labels *) * model->activities_size);
		l = malloc(sizeof(Labels));
		l->activity = activity;
		l->states_size = 0;
		l->states = NULL;

		model->labels[model->activities_size - 1] = l;
	}

	return l;
}

static void add_state_to(State *s, Labels *l)
{
	if (s == NULL)
		return;
	l->states_size++;
	l->states = realloc(l->states, sizeof(int) * (l->states_size));
	l->states[l->states_size - 1] = s->id;
}

static State *create_state(char *activity)
{
	State *state = malloc(sizeof(State));
	state->id = 0;
	state->activity = activity;
	state->children_size = 0;
	state->children = NULL;
	state->parents_size = 0;
	state->parents = NULL;
	state->is_end = 0;
	state->cardinality = 1;
	//just keep track of how many states are created
	initial_numstates++;
	return state;
}

static int assign_id_and_collect_labels(TransitionSystem *model, State *state)
{
	if (state == NULL)
	{
		return 0;
	}

	static int seq;
	if (state->id == 0)
	{
		state->id = seq++;
		//TODO: remove hardcoded activity names
		//skip pseudo states
		if (strcmp(state->activity, "__START__") != 0 && strcmp(state->activity, "__END__") != 0)
		{
			Labels *l = get_labels_for(model, state->activity);
			add_state_to(state, l);
		}
	}


	if (state->children == NULL)
	{
		return seq;
	}

	State *child;
	for (int i = 0; i < state->children_size; i++)
	{
		child = state->children[i];
		assign_id_and_collect_labels(model, child);
	}

	return seq;
}

void add_trace(TransitionSystem *model, char **activities)
{
	char *activity = *activities++;
	State *state = create_state(activity);
	State *parent = model->pseudo_initial;
	add_child(parent, state);
	add_parent(state, parent);

	parent = state;
	while((activity = *activities++) != NULL)
	{
		state = create_state(activity);
		add_child(parent, state);
		add_parent(state, parent);
		parent = state;
	}
	parent->is_end = 1;
	add_child(parent, &PSEUDO_END);
	add_parent(&PSEUDO_END, parent);
}

static void print_state(State *state, int depth)
{
	if (state == NULL)
		return;

	printf("\n");
	for (int i = 0; i < depth; i++)
		printf("\t");
	printf("{id: %d (%d), activity: %s, children:[", state->id, state->cardinality, state->activity);
	if (state->children_size == 0)
	{
		printf("]");
		return;
	}

//	printf("\n");
	State *child;
	for (int i = 0; i < state->children_size; i++)
	{
		child = state->children[i];
		print_state(child, depth + 1);
	}
	printf("\n");
	for (int i = 0; i < depth; i++)
		printf("\t");
	printf("]");
}

void print_model(TransitionSystem *model)
{
	printf("model: {");
	print_state(model->pseudo_initial, 1);
	printf("\n}\n");
}

static BDD to_primed(TransitionSystem *model, BDD p) {
	//does not bdd_addref, the caller must add it
	BDD res = bdd_appex(model->unprimed2primed, p, bddop_and, model->unprimed_vars);
	return res;
}

BDD pre_exists(TransitionSystem *model, BDD p) {
	BDD pprime = bdd_addref(to_primed(model, p));
	BDD res = bdd_appex(model->transitions_bdd, pprime, bddop_and, model->primed_vars);
	bdd_delref(pprime);
	return res;
}

double support(TransitionSystem *model, BDD result) {
	int count = 0;
	State *pi = model->pseudo_initial;
	State *child;
	int reals = 0;
	for(int i = 0; i < pi->children_size; i++) {
		child = pi->children[i];
		if (child == NULL)
			continue;
		BDD tt = bdd_addref(bdd_and(result, model->states_bdds[child->id]));

		if (tt == model->states_bdds[child->id])
			count += child->cardinality;

		bdd_delref(tt);
		reals += child->cardinality;
	}
	return (double)count / reals;
}
TransitionSystem* create_emtpty_model(void)
{
	TransitionSystem *model = malloc(sizeof(TransitionSystem));
	model->pseudo_initial = create_state("__START__");
	model->states_size = 1;
	model->transitions = NULL;
	model->transition_size = 0;
	model->transitions_bdd = 0;

	model->labels = NULL;
	model->activities_size = 0;

	return model;
}

void create_bdds(TransitionSystem *model)
{
	merge_similar_parents(&PSEUDO_END);
//	merge_similar_children(model->pseudo_initial); //merging children spoiles the calculation of support
	model->states_size = assign_id_and_collect_labels(model, model->pseudo_initial);

	get_transitions(model, NULL, model->pseudo_initial);

	init_buddy(model->states_size);

	int state_vars = ceil_of_log2_of(model->states_size);
	int bdd_vars = state_vars * 2;
	bdd_setvarnum(bdd_vars);

	BDD unprimed2primed = bdd_addref(0);
	model->states_bdds = malloc(sizeof(int *) * model->states_size);
	model->states_primed_bdds = malloc(sizeof(int *) * model->states_size);
	model->all_states = bdd_addref(0);

	//encode states as binary functions
	for (int i = 0; i < model->states_size; i++) {
		BDD tmp = bdd_addref(1);
		BDD tmpp = bdd_addref(1);
		for (int j = 0; j < state_vars; j++) {
			int test = (i >> (state_vars - 1 - j)) & 1;
			if (test == 1) {
				tmp = f_bdd_and_with(tmp, bdd_ithvar(j));
				tmpp = f_bdd_and_with(tmpp, bdd_ithvar(j + state_vars));
			} else {
				tmp = f_bdd_and_with(tmp, bdd_nithvar(j));
				tmpp = f_bdd_and_with(tmpp, bdd_nithvar(j + state_vars));
			}
		}
		model->states_bdds[i] = bdd_addref(tmp);
		model->states_primed_bdds[i] = bdd_addref(tmpp);

		model->all_states = f_bdd_or_with(model->all_states, tmp);

		BDD tt = bdd_addref(bdd_and(tmp, tmpp));
		unprimed2primed = f_bdd_or_with(unprimed2primed, tt);
		bdd_delref(tt);
		bdd_delref(tmp);
		bdd_delref(tmpp);
	}
	model->unprimed2primed = bdd_addref(unprimed2primed);
	bdd_delref(unprimed2primed);

	//create helper of unprimed and primed variables
	BDD unprimed_vars = bdd_addref(1);
	BDD primed_vars = bdd_addref(1);
	for (int i = 0; i < state_vars; i++) {
		unprimed_vars = f_bdd_and_with(unprimed_vars, bdd_ithvar(i));
		primed_vars = f_bdd_and_with(primed_vars, bdd_ithvar(i + state_vars));
	}
	model->unprimed_vars = unprimed_vars;
	model->primed_vars = primed_vars;

	//create function for transitions
	BDD transitions_bdd = bdd_addref(0);
	for (int i = 0; i < model->transition_size; i++)
	{
		BDD tt = bdd_addref(bdd_and(model->states_bdds[model->transitions[i]->src], model->states_primed_bdds[model->transitions[i]->dest]));
		transitions_bdd = f_bdd_or_with(transitions_bdd, tt);
		bdd_delref(tt);
	}

	model->transitions_bdd = transitions_bdd;

	for (int i = 0; i < model->activities_size; i++)
	{
		Labels *l = model->labels[i];
		l->states_bdd = bdd_addref(0);
		for (int j = 0; j < l->states_size; j++)
		{
			l->states_bdd = f_bdd_or_with(l->states_bdd, model->states_bdds[l->states[j]]);
		}
	}
}
