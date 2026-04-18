#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "stack.h"
#include "nfa.h"
#include "parseregex.h"

#define MAX_NFA 0xFF

#define MAX_STATE 750
re_nfa_state_t statepool[MAX_STATE]; //need more? adjust this.
int next_free_st = 0;

re_nfa_state_t* alloc_state() {
    if (next_free_st+1 == MAX_STATE)
        return NULL;
    return &statepool[next_free_st++];
}

re_nfa_transition_t transpool[2*MAX_STATE];
int next_free_tr = 0;

re_nfa_transition_t* alloc_trans() {
    if (next_free_tr+1 == 2*MAX_STATE) {
        return NULL;
    } 
    return &transpool[next_free_tr++];
}

re_nfa_state_t* make_nfa_state(State label) {
    re_nfa_state_t* state = alloc_state();
    state->label = label;
    state->trans[0] = NULL;
    state->trans[1] = NULL;
    return state;
}

re_nfa_transition_t* make_char_transition(re_nfa_state_t* dest, char ch) {
    re_nfa_transition_t* trans = alloc_trans();
    trans->dest = dest;
    trans->data = malloc(sizeof(char)*2);
    trans->data[0] = ch;
    trans->is_epsilon = false;
    trans->is_ccl = false;
    return trans;
}

re_nfa_transition_t* make_ccl_transition(re_nfa_state_t* dest, char* ccl) {
    re_nfa_transition_t* trans = alloc_trans();
    trans->dest = dest;
    trans->data = strdup(ccl);
    trans->is_epsilon = false;
    trans->is_ccl = true;
    return trans;
}

re_nfa_transition_t* make_epsilon_transition(re_nfa_state_t* dest) {
    re_nfa_transition_t* trans = alloc_trans();
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
    re_nfa_state_t* ns = alloc_state();
    re_nfa_state_t* ts = alloc_state();
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
    re_nfa_state_t* ns = alloc_state();
    re_nfa_state_t* ts = alloc_state();
    ns->label = nextLabel();
    ts->label = nextLabel();
    ns->trans[0] = make_epsilon_transition(a->start);
    ns->trans[1] = make_epsilon_transition(b->start);
    a->accept->trans[0] = make_epsilon_transition(ts);
    b->accept->trans[0] = make_epsilon_transition(ts);
    return makeNFA(ns, ts);
}

re_nfa_t* makeKleeneNFA(re_nfa_t* nfa) {
    re_nfa_state_t* ns = alloc_state();
    re_nfa_state_t* ts = alloc_state();
    ns->label = nextLabel();
    ts->label = nextLabel();
    ns->trans[0] = make_epsilon_transition(nfa->start);
    ns->trans[1] = make_epsilon_transition(ts);
    nfa->accept->trans[0] = make_epsilon_transition(ts);
    nfa->accept->trans[1] = make_epsilon_transition(nfa->start);
    return makeNFA(ns, ts);
}

void compile(re_ast_t* ast, Stack* stack) {
    if (ast == NULL)
        return;
    if (ast->type == LIT) {
        push(stack, makeAtomicNFA(make_char_transition(NULL, ast->ch)));
    } else if (ast->type == CHCLASS) {
        push(stack, makeAtomicNFA(make_ccl_transition(NULL, ast->ccl)));      
    } else {
        switch (ast->ch) {
            case '|': {
                compile(ast->left, stack);
                compile(ast->right, stack);
                re_nfa_t* rhs = pop(stack);
                re_nfa_t* lhs = pop(stack);
                push(stack, makeAltNFA(lhs, rhs));
            } break;
            case '@': {
                compile(ast->left, stack);
                compile(ast->right, stack);
                re_nfa_t* rhs = pop(stack);
                re_nfa_t* lhs = pop(stack);
                push(stack, makeConcatNFA(lhs, rhs));
            } break;
            case '*': {
                compile(ast->left, stack);
                re_nfa_t* lhs = pop(stack);
                push(stack, makeKleeneNFA(lhs));
            } break;
            case '+': {
                compile(ast->left, stack);
                re_nfa_t* lhs = pop(stack);
                push(stack, makeConcatNFA(lhs, makeKleeneNFA(lhs)));
            } break;
            case '?': {
                compile(ast->left, stack);
                re_nfa_t* lhs = pop(stack);
                re_nfa_t* rhs = makeAtomicNFA(make_epsilon_transition(NULL));
                rhs->start->trans[0]->dest = rhs->accept;
                push(stack, makeAltNFA(lhs, rhs));
            } break;
            default: 
                break;
        }
    }
}

re_nfa_t* build(re_ast_t* ast) {
    Stack st;
    initStack(&st);
    compile(ast, &st);
    re_nfa_t* nfa = pop(&st);
    return nfa;
}

re_nfa_t* re2nfa(char* pattern) {
    re_ast_t* ast = parse(pattern);
    re_nfa_t* nfa = build(ast);
    return nfa;
}