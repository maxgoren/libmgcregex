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


MatchBounds* addBounds(int l, int r) {
    MatchBounds* mb = malloc(sizeof(MatchBounds));
    mb->start = l;
    mb->end = r;
    return mb;
}


MatchContext* makeContext() {
    MatchContext* cxt = malloc(sizeof(MatchContext));
    cxt->did_match = false;
    cxt->num_groups = 0;
    return cxt;
}


typedef struct Thread {
    re_nfa_state_t* current;
    struct Thread* previous;
    char ch;
    int pos;
} Thread;

Thread* makeThread(re_nfa_state_t* curr, Thread* prev, int pos, char ch) {
    Thread* t = malloc(sizeof(Thread));
    t->current = curr;
    t->previous = prev;
    t->ch = ch;
    t->pos = pos;
    return t;
}

void updateTagContext(MatchContext* cxt, re_nfa_state_t* state, int pos, char ch) {
    for (re_tag_t* t = state->tag; t != NULL; t = t->next) {
        if (cxt->groups[t->group] == NULL) {
            cxt->groups[t->group] = addBounds(pos, pos);
        } else {
            if (t->type == END) {
                cxt->groups[t->group]->end = pos;
            } else if (t->type == START) {
                cxt->groups[t->group]->start = pos;
            } 
        }
    }
}

OrderedSet* move(char* text, int pos, MatchContext* cxt, OrderedSet* states) {
    char ch = text[pos];
    OrderedSet* next = malloc(sizeof(OrderedSet));
    initSet(next, states->cmpfunc);
    RBIterator it;
    rb_iter_init(&it, states);
    while (!rb_iter_done(&it)) {
        Thread* th = ((Thread*)rb_iter_get(&it)->value);
        for (int i = 0; i < 2; i++) {
            re_nfa_transition_t* trans = th->current->trans[i];
            if (trans != NULL && trans->is_epsilon == false) {
                if ((trans->is_ccl && match_ccl(ch, trans)) || (trans->data[0] == ch || trans->data[0] == '.')) {
                    setAdd(next, makeThread(trans->dest, ((Thread*)rb_iter_get(&it)->value), pos, ch)); 
                    //printf("\t%d ->(%c)-> %d \n", th->current->label, ch, trans->dest->label);
                }
            }
        }
        rb_iter_next(&it);
    }
    rb_destroy(states);
    return next;
}


OrderedSet* e_closure(OrderedSet* states, MatchContext* cxt, re_nfa_t* nfa, char* text, int pos) {
    char ch = text[pos];
    OrderedSet* next = states;
    Stack ss;
    initStack(&ss);
    RBIterator it;
    rb_iter_init(&it, states);
    while (!rb_iter_done(&it)) {
        push(&ss, (Thread*)rb_iter_get(&it)->value);
        rb_iter_next(&it);
    }
    while (!empty(&ss)) {
        Thread* curr_thr = ((Thread*)pop(&ss));
        if (curr_thr->current->label == nfa->accept->label) {
            cxt->did_match = true;
            for (Thread* it = curr_thr; it != NULL; it = it->previous) {
                if (it->current->is_tagged) {
                    updateTagContext(cxt, it->current, it->pos, it->ch);
                }
                printf("{%d: %d, %c}", it->current == NULL ?-1:it->current->label, it->pos, it->ch);
            }
            printf("\n");
        }
        for (int i = 0; i < 2; i++) {
            re_nfa_transition_t* trans = curr_thr->current->trans[i];
            if (trans != NULL && trans->is_epsilon) {
                Thread* t = makeThread(trans->dest, curr_thr, pos, ch);
                if (!setContains(next, t)) {
                    //printf("\t%d ->(%s)-> %d \n", curr_thr->current->label, trans->data, trans->dest->label);
                    setAdd(next, t);
                    push(&ss,t);
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

int cmp_threads(void* l, void* r) {
    Thread* lt = (Thread*)l;
    Thread* rt = (Thread*)r;
    int cmp = cmp_states(lt->current, rt->current); 
    if (cmp == 0) {
        if (lt->pos < rt->pos) return -1;
        if (lt->pos > rt->pos) return 1;
    }
    return cmp;
}

MatchContext* match_re(re_nfa_t* nfa, char* text) {
    OrderedSet *states = malloc(sizeof(OrderedSet));
    initSet(states, &cmp_threads);
    setAdd(states, makeThread(nfa->start, NULL, 0, text[0]));
    MatchContext* cxt = makeContext();
    states = e_closure(states,cxt, nfa, text, 0);
    int first_match = -1, last_match = -1;
    int i = 0;
    Thread* matched = NULL;
    for (i = 0; text[i] != '\0'; i++) {
        states = move(text, i,cxt, states);
        states = e_closure(states,cxt, nfa, text, i);
        if (setEmpty(states))
            break;
    }
    rb_destroy(states);
    return cxt;
}