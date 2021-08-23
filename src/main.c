#define _GNU_SOURCE

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "include/ast.h"
#include "include/eval.h"
#include "include/parse.h"
#include "include/run.h"
#include "include/utils.h"

/**
 **  ./myfind [options] [files] [expressions]
 */
int main(int argc, char *argv[])
{
    char **files;
    struct info **tokens;
    int opt_d = 0;
    int index = 1;
    int result = 0;
    int nb_files = 0;
    int nb_tokens = 0;

    // parsing options
    enum OPT opt = parse_opt(argv, argc, &index, &opt_d);

    // parsing files
    files = parse_arg(argv, argc, &index, &nb_files);

    // parsing expressions
    tokens = parse_tokens(argv, argc, &index, &nb_tokens);
    for (int i = 0; i < nb_tokens; i++)
    {
        if (tokens[i]->expr == DELETE)
        {
            opt_d = 1;
            break;
        }
    }

    // create ast
    struct ast *ast = create_ast(tokens, nb_tokens);

    // evaluating ast
    struct opts opts = { opt_d, opt };
    for (int i = 0; i < nb_files; i++)
        result |= run(files[i], "", opts, ast);

    free_ast(ast);
    free(files);
    free(tokens);

    return result;
}