#define _POSIX_C_SOURCE 200112L

#include "include/check.h"

#include <err.h>
#include <grp.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "include/ast.h"
#include "include/parse.h"
#include "include/utils.h"

/**
 ** Converts '-type [arg]' to TYPE enum
 */
static enum TYPE get_dir_type(char *type)
{
    enum TYPE dir_type;

    if (equals(type, "b"))
        dir_type = TYPE_B;
    else if (equals(type, "c"))
        dir_type = TYPE_C;
    else if (equals(type, "d"))
        dir_type = TYPE_D;
    else if (equals(type, "f"))
        dir_type = TYPE_F;
    else if (equals(type, "l"))
        dir_type = TYPE_L;
    else if (equals(type, "p"))
        dir_type = TYPE_P;
    else if (equals(type, "s"))
        dir_type = TYPE_S;
    else
        errx(1, "invalid file type: %s (parsing failed)", type);

    return dir_type;
}

/**
 ** Checks whether the permission of '-perm [arg]' is a valid one
 */
static int is_valid_perm(char *perm)
{
    size_t count = 0;
    if (*perm == '-' || *perm == '/')
        perm++;
    while (*perm == '0')
    {
        count += !count ? 1 : 0;
        perm++;
    }

    for (; *perm; perm++, count++)
    {
        if (!is_octal(*perm) || count > 4)
            return 0;
    }

    return !count ? 0 : 1;
}

/**
 ** Called when '-exec' or '-execdir' is met in command line args
 ** Concatenates each arg to build a single string command
 */
void check_exec(struct info **info, char **argv, int argc, int *ind,
                enum EXPR expr)
{
    (*info)->expr = expr;
    char **cmd = &(*info)->data.value;

    *cmd = calloc(1, sizeof(char));
    (*ind)++; // skip right after -exec

    while (*ind < argc && !equals(argv[*ind], ";"))
    {
        size_t prev_len = strlen(*cmd);
        size_t arg_len = strlen(argv[*ind]);

        // allocating enough space to add previous and new args + one space
        *cmd = realloc(*cmd, prev_len + arg_len + 2);

        // concatenating args from command line to assemble command
        *cmd = strcat(*cmd, argv[*ind]);
        *cmd = strcat(*cmd, " ");

        (*ind)++;
    }

    if (*ind >= argc)
        errx(1, "missing argument to '-exec'");
}

/**
 ** Called for '-name', '-perm', '-newer', '-user' and '-group' tests
 */
void check_default(struct info **info, char **argv, int argc, int *ind,
                   enum EXPR expr)
{
    if (*ind + 1 >= argc)
        errx(1, "missing argument to '%s'", get_enum_str(expr));
    else
    {
        (*info)->expr = expr;
        (*info)->data.value = argv[++(*ind)];
        char *str = (*info)->data.value;

        struct stat *f_stat = malloc(sizeof(struct stat));
        if (expr == NEWER && lstat(str, f_stat))
            errx(1, "'%s': No such file or directory", str);
        else if (expr == PERM && !is_valid_perm(str))
            errx(1, "invalid mode '%s'", str);
        else if (expr == USER && !getpwnam(str))
            errx(1, "'%s' is not the name of a known user", str);
        else if (expr == GROUP && !getgrnam(str))
            errx(1, "'%s' is not the name of an existing group", str);

        free(f_stat);
    }
}

/**
 ** Called when '-type' is met in command line args
 */
void check_type(struct info **info, char **argv, int argc, int *ind)
{
    if (*ind + 1 >= argc)
        errx(1, "missing argument to '-type'");
    else
    {
        (*info)->expr = DIR_TYPE;
        (*info)->data.dir_type = get_dir_type(argv[++(*ind)]);
    }
}