#ifndef CHECK_H
#define CHECK_H

#include "ast.h"

void check_exec(struct info **info, char **argv, int argc, int *ind,
                enum EXPR expr);
void check_default(struct info **info, char **argv, int argc, int *ind,
                   enum EXPR expr);
void check_type(struct info **info, char **argv, int argc, int *ind);

#endif