#include "parseregex.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

re_ast_t* make_ast_node(NodeType type) {
    re_ast_t* ast = malloc(sizeof(re_ast_t));
    if (ast == NULL) {
        printf("Error: out of memory\n");
        return NULL;
    }
    ast->type = type;
    ast->left = NULL;
    ast->right = NULL;
    return ast;
}

re_ast_t* make_op_node(char ch) {
    re_ast_t* node = make_ast_node(OP);
    node->ch = ch;
    return node;
}

re_ast_t* make_char_node(char ch) {
    re_ast_t* node = make_ast_node(LIT);
    node->ch = ch;
    return node;
}

re_ast_t* make_ccl_node(char* ccl) {
    re_ast_t* node = make_ast_node(CHCLASS);
    node->ccl = strdup(ccl);
    return node;
}

void print_ast(re_ast_t* ast, int d) {
    if (ast != NULL) {
        print_ast(ast->left, d+1);
        for (int i = 0; i < d; i++) printf(" ");
        if (ast->type == CHCLASS) {
            printf("%s\n", ast->ccl);
        } else {
            printf("%c\n", ast->ch);
        }
        print_ast(ast->right, d+1);
    }
}

void free_ast(re_ast_t* ast) {
    if (ast != NULL) {
        free_ast(ast->left);
        free_ast(ast->right);
        free(ast);
    }
}


typedef struct ParseStr_t {
    char* data;
    int len;
    int pos;
} ParseStr_t;

char lookahead(ParseStr_t* str) {
    return str->data[str->pos];
}

bool done(ParseStr_t* str) {
    return str->pos >= str->len || str->data[str->pos] == '\0';
}

void advance(ParseStr_t* str) {
    str->pos++;
}

bool expect(ParseStr_t* str, char ch) {
    return lookahead(str) == ch;
}

bool match(ParseStr_t* str, char ch) {
    if (expect(str, ch)) {
        advance(str);
        return true;
    }
    printf("Error: mismatched char: %c\n", ch);
    return false;
}

bool is_digit(char c) {
    return (int)c > 47 && (int)c < 58;
}

bool is_char(char c) {
    return ((int)c > 64 && (int)c < 91) || 
           ((int)c > 96 && (int)c < 123) || 
           (c == '.');
}

bool is_special(char c) {
    return (((int)c >= 32 && c < 48) || 
            ((int)c > 57 && (int)c < 65)) || 
            (((int)c > 90 && (int)c < 97) || 
            ((int)c > 122 && (int)c < 127));
}

re_ast_t* factor(ParseStr_t* str);
re_ast_t* term(ParseStr_t* str);
re_ast_t* expr(ParseStr_t* str);

re_ast_t* factor(ParseStr_t* str) {
    re_ast_t* node;
    if (expect(str, '(')) {
        match(str, '(');
        node = expr(str);
        match(str, ')');
    } else if (expect(str, '[')) {
        char* ccl;
        int lend = str->pos;
        while (!done(str) && !expect(str, ']')) lend++;
        int len = lend-str->pos;
        ccl = malloc(sizeof(char)*(len+1));
        strncpy(ccl, str->data+str->pos, len);
        str->pos = lend;
        node = make_ccl_node(ccl);
    } else if (is_digit(lookahead(str)) || is_char(lookahead(str)) || expect(str, '.')) {
        node = make_char_node(lookahead(str));
        advance(str);
    }

    if (expect(str, '*') || expect(str, '+') || expect(str, '?')) {
        re_ast_t* t = make_op_node(lookahead(str));
        t->left = node;
        node = t;
        advance(str);
    }
    return node;
}

re_ast_t* term(ParseStr_t* str) {
    re_ast_t* node = factor(str);
    if (expect(str, '(' ) || expect(str, '.') || expect(str, '[') || is_digit(lookahead(str)) || is_char(lookahead(str))) {
        re_ast_t* t = make_op_node('@');
        t->left = node;
        t->right = term(str);
        node = t;
    }
    return node;
}

re_ast_t* expr(ParseStr_t* str) {
    re_ast_t* node = term(str);
    if (expect(str, '|')) {
        re_ast_t* t = make_op_node('|');
        match(str, '|');
        t->left = node;
        t->right = expr(str);
        node = t;
    }
    return node;
}

re_ast_t* parse(char* regex) {
    ParseStr_t str;
    str.data = regex;
    str.pos = 0;
    str.len = strlen(regex);
    return expr(&str);
}