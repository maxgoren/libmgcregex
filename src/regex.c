#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parseregex.h"
#include "pattern_match.h"
#include "nfa.h"

MatchContext* match_text(char* pattern, char* text) {
    if (pattern == NULL || text == NULL) {
        return false;
    }
    re_nfa_t* nfa = re2nfa(pattern);
    MatchContext* result = match_re(nfa, text);
    return result;
}

bool simple_match(char* pattern, char* text) {
    return match_text(pattern, text)->did_match;
}

