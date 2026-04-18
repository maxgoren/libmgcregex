#ifndef rb_h
#define rb_h
#include <stdlib.h>
#include <stdio.h>
typedef int rbcolor;

typedef struct rbentry {
    void* key;
    void* value;
} rbentry;

typedef struct rbnode_ {
    rbentry* data;
    rbcolor color;
    int size;
    struct rbnode_* left;
    struct rbnode_* right;
} rbnode;

typedef struct RBIterator {
    int sp;
    rbnode* root;
    rbnode** path;
} RBIterator;

typedef struct RedBlackBST {
    rbnode* root;
    int (*cmpfunc)(void*,void*);
} RedBlackBST;

//search tree
void rb_init(RedBlackBST* rbt, int (*cmpfunc)(void*,void*));
void rb_destroy(RedBlackBST* rbt);
RedBlackBST* rb_clone(RedBlackBST* rbst);
int rb_size(RedBlackBST* rbt);
void rb_erase(RedBlackBST* rbst, void* key);
void rb_insert(RedBlackBST* rbst, void* key, void* value);
rbentry* rb_search(RedBlackBST* rbt, void* key);
void rb_for_each(RedBlackBST* rbt, void (*callback)(rbentry*));

//iterator
void rb_iter_init(RBIterator* iter, RedBlackBST* rbt);
rbentry* rb_iter_get(RBIterator* iter);
void rb_iter_next(RBIterator* iter);
int rb_iter_done(RBIterator* iter);

#endif