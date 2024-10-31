#define _POSIX_C_SOURCE 200809L

#include "eval.h"

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "ast.h"

bool *eval(const char *path, const struct ast *ast)
{
    if (!ast)
    {
        warnx("eval: ast is empty");
        return false;
    }

    for (size_t i = 0; i < sizeof(assoc) / sizeof(assoc[0]); ++i)
        if (ast->type == assoc[i].type == 0)
            return assoc[i].fun(path, ast);

    warnx("eval: Invalid type '%d'", type);
    return false;
}

int lsdir(const char *path, const struct ast *ast)
{
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

    if (eval(path, ast))
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

        if (lsdir(subpath, ast) != PASS)
            errn = FAIL;
    }

    closedir(dp);

    return errn;
}
