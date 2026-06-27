#include <stdlib.h>
#include <stdio.h>
#include "rb.h"

rbentry* makeRBEntry(void* key, void* value) {
    rbentry* ent = malloc(sizeof(rbentry));
    if (ent == NULL) {
        printf("Malloc failed\n");
        return NULL;
    }
    ent->key = key;
    ent->value = value;
    return ent;
}


rbnode* makeRBNode(void* key, void* value) {
    rbnode* node = malloc(sizeof(rbnode));
    if (node == NULL) {
        printf("Malloc failed\n");
        return NULL;
    }
    node->data = makeRBEntry(key, value);
    node->color = 1;
    node->size = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeRBNode(rbnode* node) {
    if (node != NULL) {
        freeRBNode(node->left);
        freeRBNode(node->right);
        if (node->data)
            free(node->data);
        free(node);
    }
}

void rb_init(RedBlackBST* rbt, int (*cmpfunc)(void*,void*)) {
    rbt->root = NULL;
    rbt->cmpfunc = cmpfunc;
}

void rb_destroy(RedBlackBST* rbt) {
    if (rbt == NULL)
        return;
    if (rbt->root != NULL)
        freeRBNode(rbt->root);
}

int size(rbnode* node) {
    return node == NULL ? 0:node->size;
}

int rb_size(RedBlackBST* rbt) {
    return size(rbt->root);
}

int red(rbnode* node) {
    return node == NULL ? 0:node->color;
}

int black(rbnode* node) {
    return !red(node);
}

rbnode* rotateLeft(rbnode* node) {
    rbnode* x = node->right; node->right = x->left; x->left = node;
    x->color = node->color;
    node->color = 1;
    x->size = 1 + size(x->left) + size(x->right);
    node->size = 1 + size(node->left) + size(node->right);
    return x;
}

rbnode* rotateRight(rbnode* node) {
    rbnode* x = node->left; node->left = x->right; x->right = node;
    x->color = node->color;
    node->color = 1;
    x->size = 1 + size(x->left) + size(x->right);
    node->size = 1 + size(node->left) + size(node->right);
    return x;
}

rbnode* flipColors(rbnode* node) {
    if (node != NULL)
        node->color = !node->color;
    if (node->left != NULL)
        node->left->color = !node->left->color;
    if (node->right != NULL) 
        node->right->color = !node->right->color;
    return node;
}

rbnode* fixInsert(rbnode* node) {
    if (red(node->left) && red(node->right)) {
        node = flipColors(node);
    } else if (red(node->left)) {
        if (red(node->left->left)) {
            node = rotateRight(node);
        } else if (red(node->left->right)) {
            node->left = rotateLeft(node->left);
            node = rotateRight(node);
        }
    } else if (red(node->right)) {
        if (red(node->right->right)) {
            node = rotateLeft(node);
        } else if (red(node->right->left)) {
            node->right = rotateRight(node->right);
            node = rotateLeft(node);
        }
    }
    return node;
}

rbnode* _insertRB(rbnode* node, void* key, void* value,  int (*cmpfunc)(void*,void*)) {
    if (node == NULL)
        return makeRBNode(key, value);
    int cmp = cmpfunc(key, node->data->key);
    if (cmp < 0) {
        node->left = _insertRB(node->left, key, value, cmpfunc);
    } else if (cmp > 0) {
        node->right = _insertRB(node->right, key, value, cmpfunc);
    } else {
        node->data->key = key;
        node->data->value = value;
    }
    node->size = 1 + size(node->left) + size(node->right);
    return fixInsert(node);
}

rbnode* min_node(rbnode* node) {
    if (node == NULL)
        return node;
    rbnode* x = node;
    while (x->left != NULL) x = x->left;
    return x;
}

rbnode* max_node(rbnode* node) {
    if (node == NULL)
        return node;
    rbnode* x = node;
    while (x->right != NULL) x = x->right;
    return x;
}

rbnode* pushLeft(rbnode* node) {
    node = flipColors(node);
    if (red(node->right) && red(node->right->left)) {
        node->right = rotateRight(node->right);
        node = rotateLeft(node);
        node = flipColors(node);
    } else if (red(node->right) && red(node->right->right)) {
        node = rotateLeft(node);
        node = flipColors(node);
    }
    return node;
}

rbnode* pushRight(rbnode* node) {
    node = flipColors(node);
    if (node->left) {
        if (red(node->left) && red(node->left->right)) {
            node->left = rotateLeft(node->left);
            node = rotateRight(node);
            node = flipColors(node);
        } else if (red(node->left) && red(node->left->left)) {
            node = rotateRight(node);
            node = flipColors(node);
        }
    } 
    return node;
}

rbnode* pushRedDown(rbnode* node, void* key, int (*cmpfunc)(void*, void*)) {
    if (node == NULL)
        return NULL;
    rbnode* x, *s;
    if (cmpfunc(key, node->data->key) < 0) {
        x = node->left;
        s = node->right;
    } else {
        x = node->right;
        s = node->left;
    }
    if (black(x) && red(s)) {
        node = (cmpfunc(key, node->data->key) < 0) ? rotateLeft(node):rotateRight(node);
    }
    x = cmpfunc(key, node->data->key) < 0 ? node->left:node->right;
    if (x && black(x) && black(x->left) && black(x->right)) {
        node =  (cmpfunc(key, node->data->key) < 0) ? pushLeft(node):pushRight(node);
    }
    return node;
}

rbnode* _eraseRB(rbnode* node, void* key, int (*cmpfunc)(void*, void*)) {
    if (node == NULL)
        return NULL;
    node = pushRedDown(node, key, cmpfunc);
    int cmp = cmpfunc(key, node->data->key);
    if (cmp < 0) {
        node->left = _eraseRB(node->left, key, cmpfunc);
    } else if (cmp > 0) {
        node->right = _eraseRB(node->right, key, cmpfunc);
    } else {
        if (node->right == NULL) {
            rbnode* t = node;
            node = node->left;
            free(t);
        } else {
            rbnode* tmp = min_node(node->right);
            node->data = tmp->data;
            node->right = _eraseRB(node->right, node->data->key, cmpfunc);
        }
    }
    return node;
}

rbnode* _searchRB(rbnode* node, void* key, int (*cmpfunc)(void*, void*)) {
    if (node == NULL)
        return NULL;
    int cmp = cmpfunc(key, node->data->key);
    if (cmp < 0) {
       return _searchRB(node->left, key, cmpfunc);
    } else if (cmp > 0) {
       return _searchRB(node->right, key, cmpfunc);
    } else {
        return node;
    }
}

void rb_erase(RedBlackBST* rbst, void* key) {
    if (rbst->root == NULL)
        return;
    if (black(rbst->root->left) && black(rbst->root->right))
        rbst->root->color = 1;
    rbst->root = _eraseRB(rbst->root, key, rbst->cmpfunc);
    if (rbst->root != NULL) rbst->root->color = 0;
}

void rb_insert(RedBlackBST* rbst, void* key, void* value) {
    rbst->root = _insertRB(rbst->root, key, value, rbst->cmpfunc);
    rbst->root->color = 0;
}

rbentry* rb_search(RedBlackBST* rbt, void* key) {
    rbnode* node = _searchRB(rbt->root, key, rbt->cmpfunc);
    return node == NULL ? NULL:node->data;
}

rbentry* rb_min(RedBlackBST* rbt) {
    rbnode* node = min_node(rbt->root);
    return node == NULL ? NULL:node->data;
}

rbentry* rb_max(RedBlackBST* rbt) {
    rbnode* node = max_node(rbt->root);
    return node == NULL ? NULL:node->data;
}

int max(int a, int b) {
    return (a > b) ? a:b;
}

int rb_height(rbnode* node) {
    if (node == NULL)
        return 0;
    return 1 + max(rb_height(node->left), rb_height(node->right));
}

void rb_iter_init(RBIterator* iter, RedBlackBST* rbt) {
    iter->root = rbt->root;
    iter->path = malloc(sizeof(rbnode*)*(rb_height(iter->root)+1));
    iter->sp = 0;
    rbnode* t = iter->root;
    while (t != NULL) {
        iter->path[iter->sp] = t;
        t = t->left;
        iter->sp += 1;
    }
}

rbentry* rb_iter_get(RBIterator* iter) {
    return iter->path[iter->sp-1]->data;
}

void rb_iter_next(RBIterator* iter) {
    rbnode* t = iter->path[--iter->sp];
    t = t->right;
    while (t != NULL) {
        iter->path[iter->sp] = t;
        t = t->left;
        iter->sp += 1;
    }
}

int rb_iter_done(RBIterator* iter) {
    return iter->sp == 0;
}

void rb_for_each(RedBlackBST* rbt, void (*callback)(rbentry*)) {
    RBIterator iter;
    for (rb_iter_init(&iter, rbt); !rb_iter_done(&iter); rb_iter_next(&iter)) {
        callback(rb_iter_get(&iter));
    }
}

rbnode* copy_rb_tree(rbnode* node) {
    if (node == NULL)
        return NULL;
    rbnode* nn = makeRBNode(node->data->key, node->data->value);
    nn->color = node->color;
    nn->size  = node->size;
    nn->left  = copy_rb_tree(node->left);
    nn->right = copy_rb_tree(node->right);
    return nn;
}

RedBlackBST* rb_clone(RedBlackBST* og) {
    RedBlackBST* rbst = malloc(sizeof(RedBlackBST));
    rb_init(rbst, og->cmpfunc);
    rbst->root = copy_rb_tree(rbst->root);
    return rbst;
}
