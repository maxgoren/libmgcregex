#ifndef regex_h
#define regex_h
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parseregex.h"
#include "pattern_match.h"

bool simple_match(char* pattern, char* text);

MatchContext* match_text(char* pattern, char* text);


#endif