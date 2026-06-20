#ifndef pattern_match_h
#define pattern_match_h
#include <stdbool.h>
#include "nfa.h"

typedef struct MatchBounds {
    int start;
    int end;
} MatchBounds;


#define MAX_CAPTURE 25

typedef struct MatchContext {
    bool did_match;
    int num_groups;
    MatchBounds groups[MAX_CAPTURE];
} MatchContext;

MatchContext* match_re(re_nfa_t* nfa, char* text);

#endif