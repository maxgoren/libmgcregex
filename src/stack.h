#ifndef stack_h
#define stack_h
#include <stdlib.h>

typedef struct snode_ {
    struct snode_* next;
    void* data;
} snode;

typedef struct Stack {
    snode* head;
    size_t count;
} Stack;

void initStack(Stack* st);
void push(Stack* st, void* data);
void* pop(Stack* st);
void* peek(Stack* st);
int empty(Stack* st);

#endif