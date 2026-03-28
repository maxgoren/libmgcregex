#ifndef state_set_h
#define state_set_h
#include "parseregex.h"
#include "nfa.h"

typedef struct set_node_ {
    re_nfa_state_t* data;
    struct set_node_* next;
} set_node;

set_node* make_set_node(re_nfa_state_t* data, set_node* set);
set_node* set_find(set_node* set, re_nfa_state_t* data);
set_node* set_add(set_node* set, re_nfa_state_t* data);
void free_state_set(set_node* set);
int set_size(set_node* set);

#endif