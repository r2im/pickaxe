#ifndef TRANSITION_SYSTEM_H
#define TRANSITION_SYSTEM_H

struct state
{
	int id;
	char *activity;
	int children_size;
	struct state **children;
	int parents_size;
	struct state **parents;

	int is_end;
	int cardinality;
};

struct transition
{
	int src;
	int dest;
};

struct labels
{
	char *activity;
	int states_size;
	int *states;

	BDD states_bdd;
};

typedef struct labels Labels;
typedef struct state State;
typedef struct transition Transition;

struct transition_system
{
	State *pseudo_initial;
	int states_size;
	BDD *states_bdds;
	BDD *states_primed_bdds;
	BDD all_states;

	Transition **transitions;
	int transition_size;
	BDD transitions_bdd;
	BDD unprimed2primed;
	BDD unprimed_vars;
	BDD primed_vars;

	Labels **labels;
	int activities_size;
};

typedef struct transition_system TransitionSystem;

//State PSEUDO_END = {.id = 0, .activity = "__END__", .children = NULL, .children_size = 0};

//functions
TransitionSystem* create_emtpty_model(void);
void add_trace(TransitionSystem *model, char **activities);
void print_model(TransitionSystem *model);
void create_bdds(TransitionSystem *model);
//BDD to_primed(TransitionSystem *model, BDD p);
BDD pre_exists(TransitionSystem *model, BDD p);
double support(TransitionSystem *model, BDD result);
Labels* get_labels_for(TransitionSystem *model, char *activity);

#endif /* TRANSITION_SYSTEM_H */
