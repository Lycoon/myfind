#include "include/parse.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/ast.h"
#include "include/check.h"
#include "include/utils.h"

static int is_test(enum EXPR expr)
{
    return expr >= 5;
}

static int is_op(enum EXPR expr)
{
    return expr == OR || expr == AND;
}

/**
 ** Retrieve options from command line
 */
enum OPT parse_opt(char **argv, int argc, int *ind, int *opt_d)
{
    enum OPT opt = OPT_P;
    for (; *ind < argc; (*ind)++)
    {
        if (argv[*ind][0] == '-')
        {
            char c = argv[*ind][1];
            if (c == 'd')
                *opt_d = 1;
            else if (c == 'H')
                opt = OPT_H;
            else if (c == 'L')
                opt = OPT_L;
            else if (c == 'P')
                opt = OPT_P;
            else
                return OPT_ERROR;
        }
        else
            break;
    }
    return opt;
}

/**
 ** Parses input files from command line
 */
char **parse_arg(char **argv, int argc, int *ind, int *nb_files)
{
    *nb_files = 0;
    char **files = calloc(argc - *ind + (argc == *ind ? 1 : 0), sizeof(char *));
    for (; *ind < argc; (*ind)++, (*nb_files)++)
    {
        char c = argv[*ind][0];
        if (c == '(' || c == '!' || c == '-')
            break;
        files[*nb_files] = argv[*ind];
    }

    if (*nb_files != 0)
        return files;

    files[*nb_files] = "."; // if no provided file
    (*nb_files)++;

    return files;
}

/**
 ** Evaluates whether current token is valid according to the previous one
 */
static void is_valid(enum EXPR curr, enum EXPR prev)
{
    if (curr == CLOSE_PAR)
    {
        // if empty parentheses
        if (prev == OPEN_PAR)
            errx(1, "invalid expression: empty parentheses are not allowed.");
        // if operator followed by close parenthesis
        else if (is_op(prev) || prev == NOT)
            errx(1, "expected an expression between %s and %s",
                 get_enum_str(prev), get_enum_str(curr));
    }
    else if (is_op(curr))
    {
        // if two consecutive operators
        if (is_op(prev) || prev == OPEN_PAR || prev == NOT)
            errx(1, "you have used %s with nothing before it.",
                 get_enum_str(curr));
    }
}

/**
 ** Evaluates whether there is an implicit 'AND' operator from
 ** current and previous tokens
 */
static void check_implicit_and(struct info *info, struct info *prev,
                               struct info ***tokens, int *nb_tokens)
{
    if ((is_test(info->expr) || info->expr == NOT || info->expr == OPEN_PAR)
        && (is_test(prev->expr) || prev->expr == CLOSE_PAR))
    {
        *tokens = realloc(*tokens, (*nb_tokens + 2) * sizeof(struct info *));
        struct info *and_op = malloc(sizeof(struct info));
        and_op->expr = AND;

        (*tokens)[*nb_tokens] = and_op;
        (*nb_tokens)++;
        (*tokens)[*nb_tokens] = info;
    }
}

/**
 ** Parses tokens (expressions) from command line
 */
struct info **parse_tokens(char **argv, int argc, int *ind, int *nb_tokens)
{
    struct info **tokens = NULL;
    for (; *ind < argc; (*ind)++, (*nb_tokens)++)
    {
        struct info *info = get_info(argv, argc, ind);
        if (*nb_tokens == 0)
        {
            if (is_op(info->expr)) // if first token is an operator then error
                errx(1, "expression cannot begin with %s",
                     get_enum_str(info->expr));
            tokens = realloc(tokens, (*nb_tokens + 1) * sizeof(struct info *));
            tokens[*nb_tokens] = info;
            continue;
        }

        struct info *prev = tokens[*nb_tokens - 1];

        // insert 'and' operator when needed
        check_implicit_and(info, prev, &tokens, nb_tokens);

        // crash if current token is invalid
        is_valid(info->expr, prev->expr);

        // default case
        tokens = realloc(tokens, (*nb_tokens + 1) * sizeof(struct info *));
        tokens[*nb_tokens] = info;
    }

    // last token cannot be an operator neither
    if (*nb_tokens > 0)
    {
        enum EXPR last = tokens[*nb_tokens - 1]->expr;
        if (last == OPEN_PAR || last == NOT || is_op(last))
            errx(1, "expression cannot end with %s", get_enum_str(last));
    }

    return tokens;
}

/**
 ** Parses arguments from command line and returns a struct info for AST node
 */
struct info *get_info(char **argv, int argc, int *ind)
{
    struct info *info = malloc(sizeof(struct info));

    if (argv[*ind][0] == '-')
    {
        char *test = argv[*ind];
        if (equals(test, "-name"))
            check_default(&info, argv, argc, ind, NAME);
        else if (equals(test, "-perm"))
            check_default(&info, argv, argc, ind, PERM);
        else if (equals(test, "-newer"))
            check_default(&info, argv, argc, ind, NEWER);
        else if (equals(test, "-type"))
            check_type(&info, argv, argc, ind);
        else if (equals(test, "-a"))
            info->expr = AND;
        else if (equals(test, "-o"))
            info->expr = OR;
        else if (equals(test, "-exec"))
            check_exec(&info, argv, argc, ind, EXEC);
        else if (equals(test, "-execdir"))
            check_exec(&info, argv, argc, ind, EXECDIR);
        else if (equals(test, "-user"))
            check_default(&info, argv, argc, ind, USER);
        else if (equals(test, "-group"))
            check_default(&info, argv, argc, ind, GROUP);
        else if (equals(test, "-delete"))
            info->expr = DELETE;
        else if (equals(test, "-print"))
            info->expr = PRINT;
        else
            errx(1, "invalid flag: %s (parsing failed)", argv[*ind]);
    }
    else if (equals(argv[*ind], "!"))
        info->expr = NOT;
    else if (equals(argv[*ind], "("))
        info->expr = OPEN_PAR;
    else if (equals(argv[*ind], ")"))
        info->expr = CLOSE_PAR;
    else
        errx(1, "invalid arg: %s (parsing failed)", argv[*ind]);

    return info;
}