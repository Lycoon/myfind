#ifndef AST_H
#define AST_H

#include "parse.h"

enum EXPR
{
    // operators
    OR,
    AND,
    NOT,
    OPEN_PAR, // open parenthesis
    CLOSE_PAR, // close parenthesis

    // tests
    NAME,
    USER,
    GROUP,
    PERM,
    NEWER,
    DIR_TYPE,
    PRINT,
    EXEC,
    EXECDIR,
    DELETE,
};

enum TYPE
{
    TYPE_B, // block mode
    TYPE_C, // character mode
    TYPE_D, // directories
    TYPE_F, // regular files
    TYPE_L, // symbolic links
    TYPE_P, // named pipes (FIFO)
    TYPE_S // sockets
};

struct info
{
    enum EXPR expr;
    union
    {
        enum TYPE dir_type;
        char *value;
    } data;
};

struct ast
{
    struct info *info;

    struct ast *left;
    struct ast *right;
};

struct ast *create_ast(struct info **tokens, int nb_tokens);
struct ast *add_node(struct ast **ast, struct info **tokens, int i);
void free_ast(struct ast *ast);
void free_info(struct info *info);
void print_enum(struct info *info);

#endif