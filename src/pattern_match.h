#ifndef pattern_match_h
#define pattern_match_h
#include <stdbool.h>
#include "nfa.h"

extern bool noisey;
typedef struct MatchBounds {
    int start;
    int end;
} MatchBounds;


#define MAX_CAPTURE 25

typedef struct Thread {
    re_nfa_state_t* current;
    struct Thread* previous;
    int id;
    char ch;
    int pos;
} Thread;

typedef struct MatchContext {
    bool did_match;
    int num_groups;
    MatchBounds groups[MAX_CAPTURE];
    Thread* matched_path;
} MatchContext;

MatchContext* match_re(re_nfa_t* nfa, char* text);

#endif