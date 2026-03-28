#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parseregex.h"
#include "pattern_match.h"


bool match_text(char* pattern, char* text) {
    if (pattern == NULL || text == NULL) {
        return false;
    }
    re_ast_t* ast = parse(pattern);
    re_nfa_t* nfa = build(ast);
    bool result = match_re(nfa, text);
    free_ast(ast);
    return result;
}
