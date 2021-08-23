#include "include/utils.h"

#include <stdlib.h>
#include <string.h>

int dec_2base(int dec, int dest_base)
{
    int res = 0;
    int tmp = 1;

    while (dec != 0)
    {
        res += (dec % dest_base) * tmp;
        dec /= dest_base;
        tmp *= 10;
    }

    return res;
}

int oct_2dec(int n)
{
    int a = n / 100;
    int b = (n - a * 100) / 10;
    int c = (n - a * 100 - b * 10);

    return a * 64 + b * 8 + c;
}

int is_octal(char c)
{
    return c >= '0' && c <= '7';
}

char *expand_cmd(char *cmd, char *path)
{
    int i = 0;
    int count = 0;
    int path_len = strlen(path);

    for (; cmd[i] != '\0'; i++)
    {
        // counting occurrences of {} placeholders
        if (strstr(&cmd[i], "{}") == &cmd[i])
        {
            count++;
            i++;
        }
    }

    char *new_cmd = malloc(i + 1 + count * (path_len - 2));
    for (i = 0; *cmd; cmd++)
    {
        // replacing if {} met
        if (strstr(cmd, "{}") == cmd)
        {
            strcpy(&new_cmd[i], path);
            i += path_len;
            cmd++;
        }
        else
            new_cmd[i++] = *cmd;
    }
    new_cmd[i] = '\0';

    return new_cmd;
}

char *get_enum_str(enum EXPR expr)
{
    if (expr == NAME)
        return "-name";
    else if (expr == DIR_TYPE)
        return "-type";
    else if (expr == EXEC)
        return "-exec";
    else if (expr == PERM)
        return "-perm";
    else if (expr == USER)
        return "-user";
    else if (expr == GROUP)
        return "-group";
    else if (expr == NEWER)
        return "-newer";
    else if (expr == AND)
        return "'and' operator";
    else if (expr == OR)
        return "'or' operator'";
    else if (expr == NOT)
        return "'not' operator'";
    else if (expr == OPEN_PAR)
        return "left parenthesis";
    else if (expr == CLOSE_PAR)
        return "close parenthesis";
    else
        return "none";
}