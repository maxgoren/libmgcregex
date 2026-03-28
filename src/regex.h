#ifndef regex_h
#define regex_h
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parseregex.h"
#include "pattern_match.h"

bool match_text(char* pattern, char* text);

#endif