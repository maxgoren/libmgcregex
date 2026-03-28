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
    printf("Processing move from: %c\n", ch);
    set_node* next = NULL;
    for (set_node* it = states; it != NULL; it = it->next) {
        for (int i = 0; i < 2; i++) {
            re_nfa_transition_t* trans = it->data->trans[i];
            if (trans != NULL && trans->is_epsilon == false) {
                if (trans->is_ccl) {
                    if (match_ccl(ch, trans)) {
                        printf("Oh yeah, %c\n", ch);
                        next = set_add(next, trans->dest);
                    }
                } else if (trans->data[0] == ch || trans->data[0] == '.') {
                    printf("match on %c from %d to %d\n", ch, it->data->label, trans->dest->label);
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
        printf("e_closure from %d\n", it->data->label);
    }
    while (st > 0) {
        re_nfa_state_t* curr = ss[--st];
        for (int i = 0; i < 2; i++) {
            re_nfa_transition_t* trans = curr->trans[i];
            if (trans != NULL && trans->is_epsilon) {
                printf("Adding eps from %d to %d\n", curr->label, trans->dest->label);
                next = set_add(next, trans->dest);
                ss[st++] = trans->dest;
            }
        }
    }
    return next;
}

bool match_re(re_nfa_t* nfa, char* text) {
    printf("Start: %d, Accept: %d\n", nfa->start->label, nfa->accept->label);
    set_node* states = NULL;
    states = set_add(states, nfa->start);
    states = e_closure(states);
    for (int i = 0; text[i] != '\0'; i++) {
        states = move(text[i], states);
        states = e_closure(states);
        if (states == NULL)
            return false;
        if (set_find(states, nfa->accept)) {
            printf("Possibly...");
        }
    }
    return set_find(states, nfa->accept);
}