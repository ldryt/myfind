#define _POSIX_C_SOURCE 200809L

#include "eval.h"

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

#include "ast.h"
#include "errn.h"
#include "options.h"

int should_process_directory(struct stat sb, struct opt opt)
{
    return (S_ISDIR(sb.st_mode)
            || (S_ISLNK(sb.st_mode) && (opt.L || (opt.H && opt.is_arg))));
}

void build_subpath(char *subpath, size_t size, const char *path,
                   const char *d_name)
{
    size_t path_len = strlen(path);
    snprintf(subpath, size, "%s%s%s", path,
             (path[path_len - 1] == '/') ? "" : "/", d_name);
}

int process_directory(const char *path, const struct ast *ast,
                      struct opt opt)
{
    DIR *dp;
    struct dirent *dt;
    char subpath[PATH_MAX];
    int errn = PASS;

    if (!(dp = opendir(path)))
        return PASS;

    while ((dt = readdir(dp)))
    {
        if (strcmp(dt->d_name, ".") == 0 || strcmp(dt->d_name, "..") == 0)
            continue;

        build_subpath(subpath, sizeof(subpath), path, dt->d_name);

        opt.is_arg = 0;
        if (lsdir(subpath, ast, opt) != PASS)
            errn = FAIL;
    }

    closedir(dp);
    return errn;
}

int lsdir(const char *path, const struct ast *ast, struct opt opt)
{
    struct stat sb;
    int errn = PASS;

    if (lstat(path, &sb))
    {
        warn("lsdir: cannot stat '%s'", path);
        return FAIL;
    }

    if (!opt.d && eval(path, ast) == 1)
        if (!opt.eval_print)
            printf("%s\n", path);

    if (should_process_directory(sb, opt))
    {
        if (process_directory(path, ast, opt) != PASS)
            errn = FAIL;
    }

    if (opt.d && eval(path, ast) == 1)
        if (!opt.eval_print)
            printf("%s\n", path);

    return errn;
}

static int eval_or(const char *path, const struct ast *ast)
{
    return eval(path, ast->data.children.left)
        || eval(path, ast->data.children.right);
}

static int eval_and(const char *path, const struct ast *ast)
{
    return eval(path, ast->data.children.left)
        && eval(path, ast->data.children.right);
}

static int eval_print(const char *path,
                      __attribute__((unused)) const struct ast *ast)
{
    printf("%s\n", path);
    return 1;
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

static int eval_name(const char *path, const struct ast *ast)
{
    int res = fnmatch(ast->data.value, get_name(path), 0) == 0;
    return res;
}

static int eval_type(const char *path, const struct ast *ast)
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

static int eval_newer(const char *path, const struct ast *ast)
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

static int eval_perm(const char *path, const struct ast *ast)
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

static int eval_user(const char *path, const struct ast *ast)
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

static int eval_group(const char *path, const struct ast *ast)
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

static int eval_not(const char *path, const struct ast *ast)
{
    return !eval(path, ast->data.children.right);
}

static int eval_delete(const char *path,
                       __attribute__((unused)) const struct ast *ast)
{
    if (remove(path) == -1)
    {
        warn("eval_delete: unable to remove %s", path);
        return 0;
    }
    return 1;
}

struct assoc_fun alist_fun[] = {
    { .type = AND, .fun = eval_and },
    { .type = OR, .fun = eval_or },
    { .type = PRINT, .fun = eval_print },
    { .type = NAME, .fun = eval_name },
    { .type = TYPE, .fun = eval_type },
    { .type = NEWER, .fun = eval_newer },
    { .type = PERM, .fun = eval_perm },
    { .type = USER, .fun = eval_user },
    { .type = GROUP, .fun = eval_group },
    { .type = NOT, .fun = eval_not },
    { .type = DELETE, .fun = eval_delete },
};

int eval(const char *path, const struct ast *ast)
{
    if (!ast)
        return 1;

    for (size_t i = 0; i < sizeof(alist_fun) / sizeof(alist_fun[0]); ++i)
        if (ast->type == alist_fun[i].type)
            return alist_fun[i].fun(path, ast);

    warnx("eval: Invalid type '%d'", ast->type);
    return 0;
}
