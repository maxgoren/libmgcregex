#include "stack.h"

void initStack(Stack* st) {
    st->count = 0;
    st->head = NULL;
}

void push(Stack* st, void* data) {
    snode* t = malloc(sizeof(snode));
    if (t == NULL)
        return;
    t->data = data;
    t->next = st->head;
    st->head = t;
    st->count++;
}
void* pop(Stack* st) {
    snode* t = st->head;
    void* ret = t->data;
    st->head = st->head->next;
    t->next = NULL;
    free(t); st->count--;
    return ret;
}
void* peek(Stack* st) {
    return st->head->data;
}

int empty(Stack* st) {
    return st->head == NULL;
}