#ifndef pattern_match_h
#define pattern_match_h
#include <stdbool.h>
#include "nfa.h"

bool match_re(re_nfa_t* nfa, char* text);

#endif