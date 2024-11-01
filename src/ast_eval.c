#define _POSIX_C_SOURCE 200809L

#include "ast_eval.h"

#include <dirent.h>
#include <err.h>
#include <fnmatch.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "errn.h"

struct assoc alist[] = {
    { .name = "-a", .type = AND, .fun = eval_and },
    { .name = "-o", .type = OR, .fun = eval_or },
    { .name = "-print", .type = PRINT, .fun = eval_print },
    { .name = "-name", .type = NAME, .fun = eval_name },
    { .name = "-type", .type = TYPE, .fun = eval_type },
    { .name = "-newer", .type = NEWER, .fun = eval_newer },
    { .name = "-perm", .type = PERM, .fun = eval_perm },
    { .name = "-user", .type = USER, .fun = eval_user },
    { .name = "-group", .type = GROUP, .fun = eval_group },
    { .name = "!", .type = NOT, .fun = eval_not },
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

int lsdir(const char *path, const struct ast *ast, const struct opt opt,
          int is_arg)
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

    if (eval(path, ast) == 1 && !opt.d)
        printf("%s\n", path);

    if (S_ISDIR(sb.st_mode)
        || (S_ISLNK(sb.st_mode) && (opt.L || (opt.H && is_arg))))
    {
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

            if (lsdir(subpath, ast, opt, 0) != PASS)
                errn = FAIL;
        }

        closedir(dp);
    }

    if (eval(path, ast) == 1 && opt.d)
        printf("%s\n", path);

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

int eval_type(const char *path, const struct ast *ast)
{
    struct stat sb;
    if (lstat(path, &sb))
    {
        warnx("eval_type: cannot stat '%s'", path);
        return 0;
    }

    switch (ast->data.value[0])
    {
    case 'f':
        return S_ISREG(sb.st_mode);
    case 'd':
        return S_ISDIR(sb.st_mode);
    case 'c':
        return S_ISCHR(sb.st_mode);
    case 'b':
        return S_ISBLK(sb.st_mode);
    case 'p':
        return S_ISFIFO(sb.st_mode);
    case 'l':
        return S_ISLNK(sb.st_mode);
    case 's':
        return S_ISSOCK(sb.st_mode);
    default:
        return 0;
    }
}

int eval_newer(const char *path, const struct ast *ast)
{
    struct stat sb1;
    struct stat sb2;

    if (stat(path, &sb1) == -1)
    {
        warnx("eval_newer: cannot stat '%s'", path);
        return 0;
    }
    if (stat(ast->data.value, &sb2) == -1)
    {
        warnx("eval_newer: cannot stat '%s'", ast->data.value);
        return 0;
    }

    if (sb1.st_mtim.tv_sec == sb2.st_mtim.tv_sec)
        return sb1.st_mtim.tv_nsec > sb2.st_mtim.tv_nsec;
    else
        return sb1.st_mtim.tv_sec > sb2.st_mtim.tv_sec;
}

int eval_perm(const char *path, const struct ast *ast)
{
    struct stat sb;
    mode_t path_mode;
    mode_t ast_mode;

    if (lstat(path, &sb))
    {
        warnx("eval_type: cannot stat '%s'", path);
        return 0;
    }

    path_mode = sb.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

    switch (ast->data.value[0])
    {
    case '-':
        ast_mode = strtol(ast->data.value + 1, NULL, 8);
        return (path_mode & ast_mode) == ast_mode;
    case '/':
        ast_mode = strtol(ast->data.value + 1, NULL, 8);
        return (path_mode & ast_mode) != 0;
    default:
        ast_mode = strtol(ast->data.value, NULL, 8);
        return path_mode == ast_mode;
    }
}

int eval_user(const char *path, const struct ast *ast)
{
    struct stat sb;
    struct passwd *path_pwd;
    struct passwd *ast_pwd;

    if (lstat(path, &sb))
    {
        warnx("eval_user: cannot stat '%s'", path);
        return 0;
    }

    if (!(path_pwd = getpwuid(sb.st_uid)))
    {
        warnx("eval_user: cannot get passwd struct from '%s'", path);
        return 0;
    }

    if (!(ast_pwd = getpwnam(ast->data.value)))
    {
        warnx("eval_type: cannot get passwd struct from '%s'", ast->data.value);
        return 0;
    }

    return strcmp(ast_pwd->pw_name, path_pwd->pw_name) == 0;
}

int eval_group(const char *path, const struct ast *ast)
{
    struct stat sb;
    struct group *path_grp;
    struct group *ast_grp;

    if (lstat(path, &sb))
    {
        warnx("eval_user: cannot stat '%s'", path);
        return 0;
    }

    if (!(path_grp = getgrgid(sb.st_gid)))
    {
        warnx("eval_user: cannot get group struct from '%s'", path);
        return 0;
    }

    if (!(ast_grp = getgrnam(ast->data.value)))
    {
        warnx("eval_type: cannot get group struct from '%s'", ast->data.value);
        return 0;
    }

    return strcmp(ast_grp->gr_name, path_grp->gr_name) == 0;
}

int eval_not(const char *path, const struct ast *ast)
{
    return !eval(path, ast->data.children.right);
}
