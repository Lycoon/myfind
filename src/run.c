#define _POSIX_C_SOURCE 200112L

#include "include/run.h"

#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "include/ast.h"
#include "include/eval.h"
#include "include/parse.h"

static struct stat *does_exist(char *path)
{
    struct stat *f_stat = malloc(sizeof(struct stat));
    if (lstat(path, f_stat))
    {
        free(f_stat);
        fprintf(stderr, "myfind: %s: file or directory not found\n", path);
        return NULL;
    }

    return f_stat;
}

void print_ast(char *path, char *file, struct ast *ast, struct stat *f_stat)
{
    int print = 0;
    if (eval_ast(path, file, ast, f_stat, &print) && !print)
        printf("%s\n", path);
}

static int check_flags(enum OPT opt, struct stat **f_stat, char *curr_path)
{
    if ((opt == OPT_L || opt == OPT_H) && S_ISLNK((*f_stat)->st_mode))
    {
        free(*f_stat);
        curr_path = strcat(curr_path, "/");
        if (!(*f_stat = does_exist(curr_path)))
            return 1;
    }

    return 0;
}

static void run_rec(char *path, struct opts opts, struct ast *ast)
{
    DIR *dir = opendir(path);
    if (!dir)
    {
        fprintf(stderr, "myfind: unreachable access to '%s'\n", path);
        return;
    }

    struct dirent *read;
    while ((read = readdir(dir))) // iterate recursively on each file
    {
        if (!equals(read->d_name, ".") && !equals(read->d_name, ".."))
            run(path, read->d_name, opts, ast);
    }
    closedir(dir);
}

/**
 ** Runs through each file for evaluating AST
 */
int run(char *path, char *file, struct opts opts, struct ast *ast)
{
    int result = 0;

    char curr_path[1024];
    strcpy(curr_path, path);
    strcat(curr_path, file);

    struct stat *f_stat = NULL;
    if (!(f_stat = does_exist(curr_path)))
        return 1;

    if (!opts.opt_d)
        print_ast(curr_path, file, ast, f_stat);

    if (check_flags(opts.opt, &f_stat, curr_path))
        return 1;

    if (opts.opt == OPT_H) // now we don't care, not handling symbolic links
        opts.opt = OPT_P;

    size_t len = strlen(curr_path) - 1;
    if (curr_path[len] == '/') // force ending string if ending slash
        curr_path[len] = '\0';

    int f_mode = f_stat->st_mode;
    if (!S_ISDIR(f_mode))
        result = 0;
    else if (!S_ISLNK(f_mode) || (opts.opt == OPT_L && S_ISLNK(f_mode)))
    {
        strcat(curr_path, "/");
        run_rec(curr_path, opts, ast);
    }

    if (opts.opt_d)
        print_ast(curr_path, file, ast, f_stat);

    free(f_stat);
    return result;
}