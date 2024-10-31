#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "ast.h"
#include "errn.h"
#include "lexer.h"
#include "parser.h"
#include "queue.h"

int lsdir(const char *path, const struct ast *ast)
{
    if (ast || !ast)
        errx(FAIL, "Not implemented yet");

    DIR *dp;
    struct dirent *dt;
    struct stat sb;
    char subpath[PATH_MAX];

    int errn = PASS;
    size_t path_len = strlen(path);

    if (lstat(path, &sb))
    {
        warn("lsdir: cannot stat '%s'", path);
        return FAIL;
    }

    printf("%s\n", path);

    if (!S_ISDIR(sb.st_mode))
        return PASS;

    if (!(dp = opendir(path)))
    {
        warn("lsdir: cannot open directory '%s'", path);
        return FAIL;
    }

    while ((dt = readdir(dp)))
    {
        if (strcmp(dt->d_name, ".") == 0 || strcmp(dt->d_name, "..") == 0)
            continue;

        snprintf(subpath, sizeof(subpath), "%s%s%s", path,
                 (path[path_len - 1] == '/') ? "" : "/", dt->d_name);

        if (lsdir(subpath, NULL) != PASS)
            errn = FAIL;
    }

    closedir(dp);

    return errn;
}

int main(int argc, char **argv)
{
    int errn = PASS;

    int expr_i = 1;
    while (expr_i < argc && argv[expr_i][0] != '-')
        expr_i++;

    struct queue *q = lex(argv + expr_i);
    if (!q)
        return FAIL;

    struct ast *ast = parse(q);
    if (!ast)
        return FAIL;

    for (int i = 1; i < expr_i; ++i)
    {
        if (lsdir(argv[i], ast) != PASS)
            errn = FAIL;
    }

    queue_destroy(q);
    ast_destroy(ast);

    return errn;
}
