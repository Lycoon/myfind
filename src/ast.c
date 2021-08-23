#include "include/ast.h"

#include <dirent.h>
#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "include/ast.h"
#include "include/parse.h"
#include "include/stack.h"

static void crash(struct stack *op_stack, struct stack *ast_stack, char *msg)
{
    destroy_stack(op_stack);
    destroy_stack(ast_stack);
    errx(1, "%s", msg);
}

static void check_parent(struct stack *op_stack, struct stack *ast_stack, int c)
{
    struct info *peek = stack_peek(op_stack);
    if (peek)
    {
        if (c && peek->expr == OPEN_PAR)
            crash(op_stack, ast_stack,
                  "one or more parentheses are not terminating");
    }
    else if (!c)
        crash(op_stack, ast_stack, "you have too many ')'");
}

static void merge_ast(struct stack **ast_stack, struct info *info)
{
    struct ast *right = NULL;
    struct ast *left = NULL;

    if (info->expr != NOT)
        right = stack_pop(ast_stack);

    left = stack_pop(ast_stack);

    struct ast *ast = malloc(sizeof(struct ast));
    ast->info = info;
    ast->right = right;
    ast->left = left;

    stack_push(ast_stack, ast);
}

/**
 ** Build the AST from each token with shunting-yard algorithm
 */
struct ast *create_ast(struct info **tokens, int nb_tokens)
{
    struct stack *op_stack = stack_init();
    struct stack *ast_stack = stack_init();

    struct info *peek;
    for (int i = 0; i < nb_tokens; i++)
    {
        struct info *info = tokens[i];
        if (info->expr == AND || info->expr == OR)
        {
            while ((peek = stack_peek(op_stack)) && peek->expr > info->expr
                   && peek->expr != OPEN_PAR)
            {
                peek = stack_pop(&op_stack);
                merge_ast(&ast_stack, peek);
            }
            stack_push(&op_stack, info);
        }
        else if (info->expr == OPEN_PAR || info->expr == NOT)
            stack_push(&op_stack, info);
        else if (info->expr == CLOSE_PAR)
        {
            while ((peek = stack_peek(op_stack)) && peek->expr != OPEN_PAR)
            {
                peek = stack_pop(&op_stack);
                merge_ast(&ast_stack, peek);
            }

            check_parent(op_stack, ast_stack, 0);
            if ((peek = stack_peek(op_stack)) && peek->expr == OPEN_PAR)
                free_info(stack_pop(&op_stack));
            free_info(info);
        }
        else // operand
        {
            struct ast *to_add = malloc(sizeof(struct ast));
            to_add->info = info;
            to_add->left = NULL;
            to_add->right = NULL;
            stack_push(&ast_stack, to_add);
        }
    }

    while (op_stack->size > 0) // if there are still operators on stack
    {
        check_parent(op_stack, ast_stack, 1);
        peek = stack_pop(&op_stack);
        merge_ast(&ast_stack, peek);
    }

    struct ast *final_ast = stack_pop(&ast_stack);
    destroy_stack(op_stack);
    destroy_stack(ast_stack);

    return final_ast; // last ast in stack
}

void free_ast(struct ast *ast)
{
    if (!ast)
        return;

    free_info(ast->info);
    free_ast(ast->left);
    free_ast(ast->right);

    free(ast);
}

void free_info(struct info *info)
{
    if (!info)
        return;

    if (info->expr == EXEC || info->expr == EXECDIR)
        free(info->data.value);
    free(info);
}