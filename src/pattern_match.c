#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "pattern_match.h"
#include "set.h"
#include "stack.h"


Thread historypool[1045];
int next_th;
bool noisey;

Thread* alloc_thread() {
    if (next_th < 1045) {
        return &historypool[next_th++];
    }
    printf("Out of memory\n");
    return NULL;
}

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



MatchContext* makeContext() {
    MatchContext* cxt = malloc(sizeof(MatchContext));
    cxt->did_match = false;
    for (int i = 0; i < MAX_CAPTURE; i++) {
        cxt->groups[i].start = -1;
        cxt->groups[i].end = -1;
    }
    return cxt;
}

Thread* makeThread(re_nfa_state_t* curr, Thread* prev, int pos, char ch) {
    Thread* t = alloc_thread();
    t->current = curr;
    t->previous = prev;
    t->id = -1;
    t->ch = ch;
    t->pos = pos;
    return t;
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
    int tcmp = cmp_states(lt->current, rt->current); 
    if (tcmp == 0) {
        if (lt->pos < rt->pos) return -1;
        if (lt->pos > rt->pos) return 1;
        if (lt->previous && rt->previous) {
            return cmp_states(lt->previous, rt->previous);
        }
    }
    return tcmp;
}

void updateTagContext(MatchContext* cxt, re_nfa_state_t* state, int pos, char ch) {
    bool shared_delim = false;
    for (re_tag_t* t = state->tag; t != NULL; t = t->next) {
        if (t->type == END && pos > cxt->groups[t->group].end) {
            cxt->groups[t->group].end = pos+1;
        } else if (t->type == START && cxt->groups[t->group].start == -1) {
            if (t->group == 0) {
                cxt->groups[t->group].start = pos;
            } else {
                cxt->groups[t->group].start = pos+(!shared_delim);
            }
        } 
        if (t->next != NULL)
            shared_delim = true;
    }
}

void updateWinner(Thread* it, MatchContext* cxt, char* text) {
    if (it != NULL) {
        updateWinner(it->previous, cxt, text);
        if (it->current->is_tagged) {
            updateTagContext(cxt, it->current, it->pos, it->ch);
            if (noisey)
                for (re_tag_t* t = it->current->tag; t != NULL; t = t->next)
                    printf("<%c%d>", t->type == START ? 'S':'E' ,t->group);
        }
        if (noisey) printf("{%d: %d, %c}", it->current == NULL ?-1:it->current->label, it->pos, it->ch);
    }
}

void move(OrderedSet* states, OrderedSet* next, MatchContext* cxt, char* text, int pos) {
    char ch = text[pos];
    if (noisey) printf("move(%d, %c)\n", pos, ch);
    RBIterator it;
    rb_iter_init(&it, states);
    while (!rb_iter_done(&it)) {
        Thread* th = ((Thread*)rb_iter_get(&it)->value);
        for (int i = 0; i < 2; i++) {
            re_nfa_transition_t* trans = th->current->trans[i];
            if (trans != NULL && trans->is_epsilon == false) {
                if ((trans->is_ccl && match_ccl(ch, trans)) || (trans->data[0] == ch || trans->data[0] == '.')) {
                    setAdd(next, makeThread(trans->dest, ((Thread*)rb_iter_get(&it)->value), pos, ch)); 
                    if (noisey) printf("\t%d ->(%c)-> %d \n", th->current->label, ch, trans->dest->label);
                }
            }
        }
        rb_iter_next(&it);
    }
    clearSet(states);
    if (noisey) printf("----------\n");
}

void e_closure(OrderedSet* states, OrderedSet* next, MatchContext* cxt, re_nfa_t* nfa, char* text, int pos) {
    char ch = text[pos];
    if (noisey) {
        printf("e_closure(%d, %c)\n", pos, ch);
    }
    Stack ss;
    initStack(&ss);
    RBIterator it;
    rb_iter_init(&it, states);
    while (!rb_iter_done(&it)) {
        push(&ss, (Thread*)rb_iter_get(&it)->value);
        setAdd(next, (Thread*)rb_iter_get(&it)->value);
        rb_iter_next(&it);
    }
    while (!empty(&ss)) {
        Thread* curr_thr = ((Thread*)pop(&ss));
        if (curr_thr->current->label == nfa->accept->label) {
            cxt->did_match = true;
            cxt->matched_path = curr_thr;
        }
        for (int i = 1; i >= 0; i--) {
            re_nfa_transition_t* trans = curr_thr->current->trans[i];
            if (trans != NULL && trans->is_epsilon) {
                Thread* t = makeThread(trans->dest, curr_thr, pos, ch);
                if (noisey) printf("\t%d ->(%s)-> %d \n", curr_thr->current->label, trans->data, trans->dest->label);
                if (!setContains(next, t)) {
                    setAdd(next, t);
                    push(&ss,t);
                } else {
                    if (noisey) printf("Replacing with new thread.\n");
                    setErase(next, t);
                    setAdd(next, t);
                }
            }
        }
    }
    clearSet(states);
    if (noisey) printf("----------\n");
}

MatchContext* match_re(re_nfa_t* nfa, char* text) {
    next_th = 0;
    int len = strlen(text);
    MatchContext* cxt = makeContext();
    OrderedSet states, next;
    initSet(&states, &cmp_threads);
    initSet(&next, &cmp_threads);
    setAdd(&states, makeThread(nfa->start, NULL, 0, text[0]));
    e_closure(&states,&next,cxt, nfa, text, 0);
    for (int i = 0; i < len-1; i++) {
        move(&next, &states, cxt, text, i);
        e_closure(&states,&next,cxt, nfa, text, i);
        if (setEmpty(&next)) {
            clearSet(&states);
            setAdd(&states, makeThread(nfa->start, NULL, i+1, text[i+1]));
            e_closure(&states, &next, cxt, nfa, text, i+1);
        }
    }
    if (cxt->did_match) {
        updateWinner(cxt->matched_path, cxt, text);
        if (noisey) printf("\n");
    }
    rb_destroy(&states);
    rb_destroy(&next);
    return cxt;
}