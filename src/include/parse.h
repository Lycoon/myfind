#ifndef PARSE_H
#define PARSE_H

#include <string.h>

#define equals(a, b) (strcmp((a), (b)) == 0 ? 1 : 0)

enum OPT
{
    OPT_D,
    OPT_H,
    OPT_L,
    OPT_P,
    OPT_ERROR
};

struct opts
{
    int opt_d;
    enum OPT opt;
};

char **parse_arg(char **argv, int argc, int *ind, int *nb_files);
enum OPT parse_opt(char **argv, int argc, int *ind, int *opt_d);
struct info *get_info(char **argv, int argc, int *ind);
struct info **parse_tokens(char **argv, int argc, int *ind, int *nb_files);

#endif