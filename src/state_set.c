#include "state_set.h"
#include "nfa.h"

set_node* make_set_node(re_nfa_state_t* data, set_node* set) {
    set_node* tmp = malloc(sizeof(set_node));
    tmp->data = data;
    tmp->next = set;
    return tmp;
}

set_node* set_find(set_node* set, re_nfa_state_t* data) {
    for (set_node* it = set; it != NULL; it = it->next) {
        if (it->data->label == data->label)
            return set;
    }
    return NULL;
}

set_node* set_add(set_node* set, re_nfa_state_t* data) {
    if (set_find(set, data) == NULL)
        set = make_set_node(data, set);
    return set;
}

void free_state_set(set_node* set) {
    if (set != NULL) {
        free_state_set(set->next);
        free(set);
    }
}

int set_size(set_node* set) {
    if (set == NULL)
        return 0;
    return 1 + set_size(set);
}
