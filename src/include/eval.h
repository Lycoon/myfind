#ifndef EVAL_UTILS_H
#define EVAL_UTILS_H

#include <sys/stat.h>

#include "ast.h"
#include "parse.h"

struct eval_args
{
    enum OPT opt;
    char *path;
    char *file;
    int *print;
};

int eval_ast(char *path, char *file, struct ast *ast, struct stat *f_stat,
             int *print);

#endif