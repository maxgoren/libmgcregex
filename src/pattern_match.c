#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "pattern_match.h"
#include "state_set.h"

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

set_node* move(char ch, set_node* states) {
    set_node* next = NULL;
    for (set_node* it = states; it != NULL; it = it->next) {
        for (int i = 0; i < 2; i++) {
            re_nfa_transition_t* trans = it->data->trans[i];
            if (trans != NULL && trans->is_epsilon == false) {
                if (trans->is_ccl) {
                    if (match_ccl(ch, trans)) {
                        next = set_add(next, trans->dest);
                    }
                } else if (trans->data[0] == ch || trans->data[0] == '.') {
                    next = set_add(next, trans->dest);
                }
            }
        }
    }
    return next;
}


set_node* e_closure(set_node* states) {
    set_node* next = states;
    re_nfa_state_t* ss[255];
    int st = 0;
    for (set_node* it = states; it != NULL; it = it->next) {
        ss[st++] = it->data;
    }
    while (st > 0) {
        re_nfa_state_t* curr = ss[--st];
        for (int i = 0; i < 2; i++) {
            re_nfa_transition_t* trans = curr->trans[i];
            if (trans != NULL && trans->is_epsilon) {
                next = set_add(next, trans->dest);
                ss[st++] = trans->dest;
            }
        }
    }
    return next;
}

bool match_re(re_nfa_t* nfa, char* text) {
    set_node* states = NULL;
    states = set_add(states, nfa->start);
    states = e_closure(states);
    bool did_find = false;
    for (int i = 0; text[i] != '\0'; i++) {
        states = move(text[i], states);
        states = e_closure(states);
        if (states == NULL)
            return false;
        if (set_find(states, nfa->accept)) {
            did_find = true;
        }
    }
    return did_find;
}