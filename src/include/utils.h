#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#include "ast.h"
#include "parse.h"

int is_octal(char c);
int dec_2base(int dec, int dest_base);
int oct_2dec(int n);
char *expand_cmd(char *cmd, char *path);
char *get_enum_str(enum EXPR expr);

#endif