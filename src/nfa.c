#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "nfa.h"
#include "parseregex.h"
#include "state_set.h"

#define MAX_NFA 0xFF

typedef struct nfa_stack_t_ {
    re_nfa_t* st[MAX_NFA];
    int sp;
} nfa_stack_t;

void push_nfa_stack(nfa_stack_t* stack, re_nfa_t* nfa) {
    stack->st[stack->sp++] = nfa;
}

re_nfa_t* pop_nfa_stack(nfa_stack_t* stack) {
    stack->sp--;
    if (stack->sp < 0) {
        puts("Stack underflow");
        return NULL;
    }
    return stack->st[stack->sp];
}

bool nfa_stack_empty(nfa_stack_t* stack) {
    return stack->sp == 0;
}


re_nfa_state_t* make_nfa_state(State label) {
    re_nfa_state_t* state = malloc(sizeof(re_nfa_state_t));
    state->label = label;
    state->trans[0] = NULL;
    state->trans[1] = NULL;
    return state;
}

re_nfa_transition_t* make_char_transition(re_nfa_state_t* dest, char ch) {
    re_nfa_transition_t* trans = malloc(sizeof(re_nfa_transition_t));
    trans->dest = dest;
    trans->data = malloc(sizeof(char)*2);
    trans->data[0] = ch;
    trans->is_epsilon = false;
    trans->is_ccl = false;
    return trans;
}

re_nfa_transition_t* make_ccl_transition(re_nfa_state_t* dest, char* ccl) {
    re_nfa_transition_t* trans = malloc(sizeof(re_nfa_transition_t));
    trans->dest = dest;
    trans->data = strdup(ccl);
    trans->is_epsilon = false;
    trans->is_ccl = true;
    return trans;
}

re_nfa_transition_t* make_epsilon_transition(re_nfa_state_t* dest) {
    re_nfa_transition_t* trans = malloc(sizeof(re_nfa_transition_t));
    trans->dest = dest;
    trans->data = strdup("&");
    trans->is_epsilon = true;
    return trans;
}
int nextLabel() {
    static int next = 0;
    return next++;
}

re_nfa_t* makeNFA(re_nfa_state_t* start, re_nfa_state_t* fin) {
    re_nfa_t* nfa = malloc(sizeof(re_nfa_t));
    nfa->start = start;
    nfa->accept = fin;
    return nfa;
}

re_nfa_t* makeAtomicNFA(re_nfa_transition_t* trans) {
    re_nfa_state_t* ns = malloc(sizeof(re_nfa_state_t));
    re_nfa_state_t* ts = malloc(sizeof(re_nfa_state_t));
    ns->label = nextLabel();
    ts->label = nextLabel();
    trans->dest = ts;
    ns->trans[0] = trans;
    return makeNFA(ns, ts);
}

re_nfa_t* makeConcatNFA(re_nfa_t* a, re_nfa_t* b) {
    a->accept->trans[0] = make_epsilon_transition(b->start);
    a->accept = b->accept;
    return a;
}

re_nfa_t* makeAltNFA(re_nfa_t* a, re_nfa_t* b) {
    re_nfa_state_t* ns = malloc(sizeof(re_nfa_state_t));
    re_nfa_state_t* ts = malloc(sizeof(re_nfa_state_t));
    ns->label = nextLabel();
    ts->label = nextLabel();
    ns->trans[0] = make_epsilon_transition(a->start);
    ns->trans[1] = make_epsilon_transition(b->start);
    a->accept->trans[0] = make_epsilon_transition(ts);
    b->accept->trans[0] = make_epsilon_transition(ts);
    return makeNFA(ns, ts);
}

re_nfa_t* makeKleeneNFA(re_nfa_t* nfa) {
    re_nfa_state_t* ns = malloc(sizeof(re_nfa_state_t));
    re_nfa_state_t* ts = malloc(sizeof(re_nfa_state_t));
    ns->label = nextLabel();
    ts->label = nextLabel();
    ns->trans[0] = make_epsilon_transition(nfa->start);
    ns->trans[1] = make_epsilon_transition(ts);
    nfa->accept->trans[0] = make_epsilon_transition(ts);
    nfa->accept->trans[1] = make_epsilon_transition(nfa->start);
    return makeNFA(ns, ts);
}

void compile(re_ast_t* ast, nfa_stack_t* stack) {
    if (ast == NULL)
        return;
    if (ast->type == LIT) {
        push_nfa_stack(stack, makeAtomicNFA(make_char_transition(NULL, ast->ch)));
    } else if (ast->type == CHCLASS) {
        push_nfa_stack(stack, makeAtomicNFA(make_ccl_transition(NULL, ast->ccl)));      
    } else {
        switch (ast->ch) {
            case '|': {
                compile(ast->left, stack);
                compile(ast->right, stack);
                re_nfa_t* rhs = pop_nfa_stack(stack);
                re_nfa_t* lhs = pop_nfa_stack(stack);
                push_nfa_stack(stack, makeAltNFA(lhs, rhs));
            } break;
            case '@': {
                compile(ast->left, stack);
                compile(ast->right, stack);
                re_nfa_t* rhs = pop_nfa_stack(stack);
                re_nfa_t* lhs = pop_nfa_stack(stack);
                push_nfa_stack(stack, makeConcatNFA(lhs, rhs));
            } break;
            case '*': {
                compile(ast->left, stack);
                re_nfa_t* lhs = pop_nfa_stack(stack);
                push_nfa_stack(stack, makeKleeneNFA(lhs));
            } break;
            case '?': {
                compile(ast->left, stack);
                compile(ast->right, stack);
                re_nfa_t* lhs = pop_nfa_stack(stack);
                re_nfa_t* rhs = makeAtomicNFA(make_epsilon_transition(NULL));
                rhs->start->trans[0]->dest = rhs->accept;
                push_nfa_stack(stack, makeAltNFA(lhs, rhs));
            } break;
            default: 
                break;
        }
    }
}

re_nfa_t* build(re_ast_t* ast) {
    nfa_stack_t* stack = malloc(sizeof(nfa_stack_t));
    stack->sp = 0;
    compile(ast, stack);
    re_nfa_t* nfa = pop_nfa_stack(stack);
    free(stack);
    return nfa;
}

