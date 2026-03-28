#include <stdio.h>
#include <stdlib.h>
#include <mgc/regex/regex.h>

void fromStdIn(char* pattern) {
    re_ast_t* ast = parse(pattern);
    print_ast(ast, 1);
    re_nfa_t* nfa = build(ast);
    char buffer[1024];
    while (fgets(buffer, 1014, stdin)) {
        if (match_re(nfa, buffer)) {
            printf("%s", buffer);
        }
    }
}

int main(int argc, char* argv[]) {
    fromStdIn(argv[1]);
    return 0;
}