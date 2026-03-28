#include <stdio.h>
#include <stdlib.h>
#include <mgc/regex/regex.h>

int main(int argc, char* argv[]) {
    if (match_text("(a*b|ac)d", "aaaabd")) {
        printf("Sho nuff.\n");
    }
    return 0;
}