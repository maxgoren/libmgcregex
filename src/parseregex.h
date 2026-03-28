#ifndef parseregex_h
#define parseregex_h

typedef enum NodeType {
    LIT, OP, CHCLASS
} NodeType;

typedef struct re_ast_t {
    NodeType type;
    union {
        char ch;
        char* ccl;
    };
    struct re_ast_t* left;
    struct re_ast_t* right;
} re_ast_t;

re_ast_t* parse(char* regex);
void free_ast(re_ast_t* ast);

#endif