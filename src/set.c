#include <stdio.h>
#include <stdlib.h>
#include "set.h"


void initSet(OrderedSet* set, int (*cmp)(void*,void*)) {
    rb_init(set, cmp);
}

void destroySet(OrderedSet* set) {
    rb_destroy(set);
}

void setAdd(OrderedSet* set, void* data) {
    rb_insert(set, data, data);
}

void setErase(OrderedSet* set, void* data) {
    rb_erase(set, data);
}

SetItem* setGetItem(OrderedSet* set, void* key) {
    return rb_search(set, key);
}

bool setContains(OrderedSet* set, void* key) {
    return setGetItem(set, key) != NULL;
}

int setSize(OrderedSet* set) {
    return rb_size(set);
}

int setEmpty(OrderedSet* set) {
    return setSize(set) == 0;
}

OrderedSet* set_union(OrderedSet* set, RBIterator* lit, RBIterator* rit) {
    while (!rb_iter_done(lit) && !rb_iter_done(rit)) {
        int cmp = set->cmpfunc(rb_iter_get(lit)->key, rb_iter_get(rit)->key);
        if (cmp < 0) {
            setAdd(set, rb_iter_get(lit)->key);
            rb_iter_next(lit);
        } else if (cmp > 0) {
            setAdd(set, rb_iter_get(rit)->key);
            rb_iter_next(rit);
        } else {
            setAdd(set, rb_iter_get(lit)->key);
            rb_iter_next(lit);
            rb_iter_next(rit);
        }
    }
    return set;
}

OrderedSet* set_difference(OrderedSet* set, RBIterator* lit, RBIterator* rit) {
    while (!rb_iter_done(lit) && !rb_iter_done(rit)) {
        int cmp = set->cmpfunc(rb_iter_get(lit)->key, rb_iter_get(rit)->key);
        if (cmp < 0) {
            setAdd(set, rb_iter_get(lit)->key);
            rb_iter_next(lit);
        } else if (cmp > 0) {
            rb_iter_next(rit);
        } else {
            rb_iter_next(lit);
            rb_iter_next(rit);
        }
    }
    return set;
}


OrderedSet* set_sym_difference(OrderedSet* set, RBIterator* lit, RBIterator* rit) {
    while (!rb_iter_done(lit) && !rb_iter_done(rit)) {
        int cmp = set->cmpfunc(rb_iter_get(lit)->key, rb_iter_get(rit)->key);
        if (cmp < 0) {
            setAdd(set, rb_iter_get(lit)->key);
            rb_iter_next(lit);
        } else if (cmp > 0) {
            setAdd(set, rb_iter_get(rit)->key);
            rb_iter_next(rit);
        } else {
            rb_iter_next(lit);
            rb_iter_next(rit);
        }
    }
    return set;
}

OrderedSet* set_intersection(OrderedSet* set, RBIterator* lit, RBIterator* rit) {
    while (!rb_iter_done(lit) && !rb_iter_done(rit)) {
        int cmp = set->cmpfunc(rb_iter_get(lit)->key, rb_iter_get(rit)->key);
        if (cmp < 0) {
            rb_iter_next(lit);
        } else if (cmp > 0) {
            rb_iter_next(rit);
        } else {
            setAdd(set, rb_iter_get(lit)->key);
            rb_iter_next(lit);
            rb_iter_next(rit);
        }
    }
    return set;
}

OrderedSet* set_merge(OrderedSet* lhs, OrderedSet* rhs, OrderedSet* (*operation)(OrderedSet*,RBIterator*,RBIterator*)) {
    OrderedSet* set = malloc(sizeof(OrderedSet));
    initSet(set, lhs->cmpfunc);
    RBIterator lit, rit;
    rb_iter_init(&lit, lhs);
    rb_iter_init(&rit, rhs);
    set = operation(set, &lit, &rit);
    while (!rb_iter_done(&lit)) { 
        setAdd(set, rb_iter_get(&lit)->key);
        rb_iter_next(&lit);
    }
    while (!rb_iter_done(&rit)) {
        setAdd(set, rb_iter_get(&rit)->key);
        rb_iter_next(&rit);
    }
    return set;
}

OrderedSet* setUnion(OrderedSet* lhs, OrderedSet* rhs) {
    return set_merge(lhs, rhs, &set_union);
}

OrderedSet* setIntersection(OrderedSet* lhs, OrderedSet* rhs) {
    return set_merge(lhs, rhs, &set_intersection);
}

OrderedSet* setDifference(OrderedSet* lhs, OrderedSet* rhs) {
    return set_merge(lhs, rhs, &set_difference);
}

OrderedSet* setSymmetricDifference(OrderedSet* lhs, OrderedSet* rhs) {
    return set_merge(lhs, rhs, &set_sym_difference);
}