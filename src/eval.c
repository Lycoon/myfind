#define _POSIX_C_SOURCE 200112L

#include "include/eval.h"

#include <dirent.h>
#include <err.h>
#include <fnmatch.h>
#include <grp.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "include/ast.h"
#include "include/parse.h"
#include "include/utils.h"

static int eval_name(char *path, char *file, struct info *info)
{
    if ((!strlen(file) && !fnmatch(info->data.value, path, FNM_PATHNAME))
        || !fnmatch(info->data.value, file, FNM_PATHNAME))
        return 1;
    return 0;
}

static int eval_type(struct info *info, struct stat *f_stat)
{
    enum TYPE f_type = info->data.dir_type;
    if (f_type == TYPE_B)
        return S_ISBLK(f_stat->st_mode) != 0 ? 1 : 0;
    else if (f_type == TYPE_C)
        return S_ISCHR(f_stat->st_mode) != 0 ? 1 : 0;
    else if (f_type == TYPE_D)
        return S_ISDIR(f_stat->st_mode) != 0 ? 1 : 0;
    else if (f_type == TYPE_F)
        return S_ISREG(f_stat->st_mode) != 0 ? 1 : 0;
    else if (f_type == TYPE_L)
        return S_ISLNK(f_stat->st_mode) != 0 ? 1 : 0;
    else if (f_type == TYPE_P)
        return S_ISFIFO(f_stat->st_mode) != 0 ? 1 : 0;
    else if (f_type == TYPE_S)
        return S_ISSOCK(f_stat->st_mode) != 0 ? 1 : 0;
    else
        return 0;
}

static int get_path(char *path)
{
    char *last = strrchr(path, '/');
    if (!last || *(last + 1) == '\0')
        return 0;

    return last - path + 1;
}

static int eval_exec(char *path, char *file, struct info *info, int *print)
{
    int ex_dir = 0;
    char dir[512] = { 0 };
    char new_path[512] = { 0 };
    if (info->expr == EXECDIR)
    {
        ex_dir = 1;
        int last = get_path(path);
        if (equals(file, ""))
            file = path + last;

        strncpy(new_path, path, last);
        strcpy(dir, "./");
        strcat(dir, file);
    }

    int pid = fork();
    if (pid == 0)
    {
        if (ex_dir)
            chdir(new_path);

        info->data.value = expand_cmd(info->data.value, ex_dir ? dir : path);
        execlp("/bin/sh", "myfind", "-c", info->data.value, NULL);
    }

    int exit_status;
    waitpid(pid, &exit_status, 0);
    *print = 1;

    return WEXITSTATUS(exit_status) != 0 ? 0 : 1;
}

static int eval_owner(struct info *info, struct stat *f_stat)
{
    if (info->expr == USER)
    {
        struct passwd *pwd = getpwuid(f_stat->st_uid);
        return equals(info->data.value, pwd->pw_name) ? 1 : 0;
    }

    struct group *grp = getgrgid(f_stat->st_gid);
    return equals(info->data.value, grp->gr_name) ? 1 : 0;
}

static int eval_perm(struct info *info, struct stat *f_stat)
{
    char *str = info->data.value;

    unsigned i = oct_2dec(atoi(str + (str[0] == '-' || str[0] == '/' ? 1 : 0)));
    unsigned mode = oct_2dec(dec_2base(f_stat->st_mode, 8) % 1000);

    if (str[0] == '-')
        return (i & mode) == i ? 1 : 0;
    else if (str[0] == '/')
        return (i & mode) > 0 ? 1 : 0;

    return i == mode ? 1 : 0;
}

static int eval_newer(struct info *info, struct stat *f_stat)
{
    struct stat curr_stat;
    if (lstat(info->data.value, &curr_stat))
        errx(1, "'%s': No such file or directory", info->data.value);

    int res = 0;
    if (f_stat->st_mtime > curr_stat.st_mtime)
        res = 1;
    else if (f_stat->st_mtime == curr_stat.st_mtime)
        if (f_stat->st_mtimensec > curr_stat.st_mtimensec)
            res = 1;

    return res;
}

static int eval_delete(char *path, int *print)
{
    int st = remove(path);
    if (st)
        warnx("cannot delete '%s': No such file or directory", path);
    *print = 1;
    return st ? 0 : 1;
}

static int eval_print(char *path, int *print)
{
    printf("%s\n", path);
    *print = 1;
    return 1;
}

int eval_ast(char *path, char *file, struct ast *ast, struct stat *f_stat,
             int *print)
{
    if (!ast)
        return 1;

    struct info *info = ast->info;
    if (info->expr == NOT)
        return !eval_ast(path, file, ast->left, f_stat, print);
    else if (info->expr == OR)
        return eval_ast(path, file, ast->left, f_stat, print)
            || eval_ast(path, file, ast->right, f_stat, print);
    else if (info->expr == AND)
        return eval_ast(path, file, ast->left, f_stat, print)
            && eval_ast(path, file, ast->right, f_stat, print);
    else if (info->expr == NAME)
        return eval_name(path, file, ast->info);
    else if (info->expr == DIR_TYPE)
        return eval_type(info, f_stat);
    else if (info->expr == NEWER)
        return eval_newer(info, f_stat);
    else if (info->expr == PERM)
        return eval_perm(info, f_stat);
    else if (info->expr == USER || info->expr == GROUP)
        return eval_owner(info, f_stat);
    else if (info->expr == EXEC || info->expr == EXECDIR)
        return eval_exec(path, file, ast->info, print);
    else if (info->expr == DELETE)
        return eval_delete(path, print);
    else if (info->expr == PRINT)
        return eval_print(path, print);

    return 0;
}
