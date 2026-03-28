#ifndef nfa_h
#define nfa_h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parseregex.h"

typedef int State;
typedef struct re_nfa_state_t_ re_nfa_state_t;

typedef struct re_nfa_transition_t_ {
    char* data;
    bool is_epsilon;
    bool is_ccl;
    re_nfa_state_t* dest;
} re_nfa_transition_t;

typedef struct re_nfa_state_t_ {
    State label;
    re_nfa_transition_t* trans[2];
} re_nfa_state_t;

typedef struct re_nfa_t_ {
    re_nfa_state_t* start;
    re_nfa_state_t* accept;
} re_nfa_t;

re_nfa_t* build(re_ast_t* ast);


#endif