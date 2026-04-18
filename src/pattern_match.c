#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "pattern_match.h"
#include "set.h"
#include "stack.h"

bool checkRange(char check, char low, char high) {
    return check >= low && check <= high;
}

bool match_ccl(char ch, re_nfa_transition_t* trans) {
    bool is_compliment = trans->data[0] == '^';
    for (int i = 0; trans->data[i]; i++) {
        if (trans->data[i] == '-' && trans->data[i+1] != '\0') {
            if (checkRange(ch, trans->data[i-1], trans->data[i+1]))
                return true;
        }
        if (trans->data[i] == ch) 
            return true;
    }
    return false;
}

OrderedSet* move(char ch, OrderedSet* states) {
    OrderedSet* next = malloc(sizeof(OrderedSet));
    initSet(next, states->cmpfunc);
    RBIterator it;
    rb_iter_init(&it, states);
    while (!rb_iter_done(&it)) {
        for (int i = 0; i < 2; i++) {
            re_nfa_transition_t* trans = ((re_nfa_state_t*)rb_iter_get(&it)->value)->trans[i];
            if (trans != NULL && trans->is_epsilon == false) {
                if (trans->is_ccl) {
                    if (match_ccl(ch, trans)) {
                        setAdd(next, trans->dest);
                    }
                } else if (trans->data[0] == ch || trans->data[0] == '.') {
                    setAdd(next, trans->dest);
                }
            }
        }
        rb_iter_next(&it);
    }
    rb_destroy(states);
    return next;
}


OrderedSet* e_closure(OrderedSet* states) {
    OrderedSet* next = states;
    Stack ss;
    initStack(&ss);
    RBIterator it;
    rb_iter_init(&it, states);
    while (!rb_iter_done(&it)) {
        push(&ss, rb_iter_get(&it)->value);
        rb_iter_next(&it);
    }
    while (!empty(&ss)) {
        re_nfa_state_t* curr = pop(&ss);
        for (int i = 0; i < 2; i++) {
            re_nfa_transition_t* trans = curr->trans[i];
            if (trans != NULL && trans->is_epsilon) {
                if (!setContains(next, trans->dest)) {
                    setAdd(next, trans->dest);
                    push(&ss,trans->dest);
                }
            }
        }
    }
    return next;
}

int cmp_states(void* l, void* r) {
    re_nfa_state_t* a = (re_nfa_state_t*)l;
    re_nfa_state_t* b = (re_nfa_state_t*)r;
    if (a->label < b->label) return -1;
    if (a->label > b->label) return 1;
    return 0;
}

bool match_re(re_nfa_t* nfa, char* text) {
    OrderedSet *states = malloc(sizeof(OrderedSet));
    initSet(states, &cmp_states);
    setAdd(states, nfa->start);
    states = e_closure(states);
    bool did_find = false;
    for (int i = 0; text[i] != '\0'; i++) {
        states = move(text[i], states);
        states = e_closure(states);
        if (setEmpty(states))
            break;
        if (setContains(states, nfa->accept)) {
            did_find = true;
        }
    }
    rb_destroy(states);
    return did_find;
}