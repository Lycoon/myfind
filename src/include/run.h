#ifndef RUN_H
#define RUN_H

#include "ast.h"
#include "parse.h"

int run(char *path, char *file, struct opts opts, struct ast *ast);

#endif