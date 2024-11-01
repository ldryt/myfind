#define _POSIX_C_SOURCE 200809L

#include "ast_eval.h"

#include <dirent.h>
#include <err.h>
#include <fnmatch.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "errn.h"

struct assoc alist[] = {
    { .name = "-a", .type = AND, .fun = eval_and },
    { .name = "-o", .type = OR, .fun = eval_or },
    { .name = "-print", .type = PRINT, .fun = eval_print },
    { .name = "-name", .type = NAME, .fun = eval_name },
};

struct ast *ast_init(const char *name)
{
    struct ast *ast = malloc(sizeof(struct ast));
    if (!ast)
    {
        warn("ast_init: Memory failure");
        return NULL;
    }

    ast->data.value = NULL;
    ast->data.children.left = NULL;
    ast->data.children.right = NULL;

    for (size_t i = 0; i < sizeof(alist) / sizeof(alist[0]); ++i)
    {
        if (strcmp(name, alist[i].name) == 0)
        {
            ast->type = alist[i].type;
            return ast;
        }
    }

    warnx("ast_init: Invalid name '%s'", name);
    free(ast);
    return NULL;
}

void ast_destroy(struct ast *ast)
{
    if (!ast)
        return;

    if (is_operator(ast))
    {
        ast_destroy(ast->data.children.right);
        ast_destroy(ast->data.children.left);
    }

    free(ast);
}

int is_operator(const struct ast *ast)
{
    return ast->type <= OPDELIM;
}

int is_action(const struct ast *ast)
{
    return ast->type > OPDELIM && ast->type <= ACTDELIM;
}

int is_test(const struct ast *ast)
{
    return ast->type > ACTDELIM;
}

// returns an integer indicating the result of the comparison, as follows:
// 0, if ast1 or ast2 has no precedence over the other;
// a positive value if ast1 precede over ast2;
// a negative value if ast2 precede over ast1;
int precedence(const struct ast *ast1, const struct ast *ast2)
{
    if (ast1->type == ast2->type)
        return 0;
    return (ast1->type > ast2->type) ? 1 : -1;
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

    if (eval(path, ast) == 1)
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

int eval(const char *path, const struct ast *ast)
{
    if (!ast)
        return 1;

    for (size_t i = 0; i < sizeof(alist) / sizeof(alist[0]); ++i)
        if (ast->type == alist[i].type)
            return alist[i].fun(path, ast);

    warnx("eval: Invalid type '%d'", ast->type);
    return 0;
}

int eval_or(const char *path, const struct ast *ast)
{
    return eval(path, ast->data.children.left)
        || eval(path, ast->data.children.right);
}

int eval_and(const char *path, const struct ast *ast)
{
    return eval(path, ast->data.children.left)
        && eval(path, ast->data.children.right);
}

int eval_print(const char *path, __attribute__((unused)) const struct ast *ast)
{
    return printf("%s\n", path);
}

static const char *get_name(const char *path)
{
    const char *end = path + strlen(path) - 1;

    while (end > path && *end == '/')
        end--;

    const char *start = end;
    while (start > path && *(start - 1) != '/')
        start--;

    return start;
}

int eval_name(const char *path, const struct ast *ast)
{
    int res = fnmatch(ast->data.value, get_name(path), 0) == 0;
    return res;
}
