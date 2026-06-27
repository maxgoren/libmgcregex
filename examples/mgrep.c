#include <stdio.h>
#include <stdlib.h>
#include <mgc/regex/regex.h>

void fromStdIn(char* pattern) {
    re_ast_t* ast = parse(pattern);
    print_ast(ast, 1);
    re_nfa_t* nfa = build(ast);
    char buffer[1024];
    while (fgets(buffer, 1014, stdin)) {
        MatchContext* cxt = match_re(nfa, buffer);
        if (cxt->did_match) {
            int i = 0;
            while (cxt->groups[i].start != -1) {
                printf("Group %d:", i);
                for (int j = cxt->groups[i].start; j < cxt->groups[i].end; j++) {
                    printf("%c", buffer[j]);
                }
                if (cxt->groups[i].start == cxt->groups[i].end) {
                    printf("%c",buffer[cxt->groups[i].start]);
                }
                printf(" (%d,%d)\n", cxt->groups[i].start, cxt->groups[i].end);
                i++;
            }
        }
    }
}
void show_use() {
    puts("egrep <pattern>");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        show_use();
        return 0;
    }
    if (argc == 2) {
        noisey = false;
        fromStdIn(argv[1]);
    } else {
        noisey = true;
        fromStdIn(argv[2]);
    }
    return 0;
}