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
#include <sys/wait.h>
#include <unistd.h>

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

int process_directory(const char *path, const struct ast *ast, struct opt opt)
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
static int handle_eval(const char *path, const struct ast *ast, struct opt opt)
{
    int res = eval(path, ast);
    if (res == -1)
        return FAIL;

    if (res == 1 && !opt.eval_print)
        printf("%s\n", path);

    return PASS;
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

    if (!opt.d && handle_eval(path, ast, opt) != PASS)
        errn = FAIL;

    if (should_process_directory(sb, opt))
    {
        if (process_directory(path, ast, opt) != PASS)
            errn = FAIL;
    }

    if (opt.d && handle_eval(path, ast, opt) != PASS)
        errn = FAIL;

    return errn;
}

/* -1 => error
 *  0 => false
 *  1 => true
 *  2 => true but do not print again
 */
static int eval_and(const char *path, const struct ast *ast)
{
    int left = eval(path, ast->data.children.left);
    if (left < 1)
        return left;

    return eval(path, ast->data.children.right);
}

static int eval_or(const char *path, const struct ast *ast)
{
    int left = eval(path, ast->data.children.left);
    if (left != 0)
        return left;

    int right = eval(path, ast->data.children.right);
    if (right != 0)
        return right;

    return 0;
}

static int eval_not(const char *path, const struct ast *ast)
{
    int res = eval(path, ast->data.children.right);
    if (res == -1)
        return -1;

    return res == 0 ? 1 : 0;
}

static int eval_print(const char *path,
                      __attribute__((unused)) const struct ast *ast)
{
    printf("%s\n", path);
    return 2;
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
    int res = fnmatch(ast->data.value, get_name(path), 0);
    if (res == 0)
        return 1;
    else if (res == FNM_NOMATCH)
        return 0;
    else
        return -1;
}

static int eval_type(const char *path, const struct ast *ast)
{
    struct stat sb;
    if (lstat(path, &sb))
    {
        warnx("eval_type: cannot stat '%s'", path);
        return -1;
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
        return -1;
    }
    if (stat(ast->data.value, &sb2) == -1)
    {
        warnx("eval_newer: cannot stat '%s'", ast->data.value);
        return -1;
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
        warnx("eval_perm: cannot stat '%s'", path);
        return -1;
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
        return -1;
    }

    if (!(path_pwd = getpwuid(sb.st_uid)))
    {
        warnx("eval_user: cannot get passwd struct from '%s'", path);
        return -1;
    }

    if (!(ast_pwd = getpwnam(ast->data.value)))
    {
        warnx("eval_user: cannot get passwd struct from '%s'", ast->data.value);
        return -1;
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
        warnx("eval_group: cannot stat '%s'", path);
        return -1;
    }

    if (!(path_grp = getgrgid(sb.st_gid)))
    {
        warnx("eval_group: cannot get group struct from '%s'", path);
        return -1;
    }

    if (!(ast_grp = getgrnam(ast->data.value)))
    {
        warnx("eval_group: cannot get group struct from '%s'", ast->data.value);
        return -1;
    }

    return strcmp(ast_grp->gr_name, path_grp->gr_name) == 0;
}

static int eval_delete(const char *path,
                       __attribute__((unused)) const struct ast *ast)
{
    if (remove(path) == -1)
    {
        warn("eval_delete: unable to remove %s", path);
        return -1;
    }
    return 1;
}

static int eval_exec(const char *path, const struct ast *ast)
{
    if (ast->data.exec->end == '+')
    {
        warnx("eval_exec: '+' terminator not supported");
        return -1;
    }

    size_t path_len = strlen(path);
    char **argv = calloc(ast->data.exec->argc + 1, sizeof(char *));
    if (!argv)
        return -1;

    for (size_t i = 0; i < ast->data.exec->argc; i++)
    {
        const char *arg = ast->data.exec->argv[i];
        size_t len = strlen(arg);
        size_t nlen = len;

        for (size_t j = 0; j < len - 1; j++)
        {
            if (arg[j] == '{' && arg[j + 1] == '}')
                nlen += path_len - 2;
        }

        char *rep = malloc(nlen + 1);
        if (!rep)
        {
            warn("eval_exec: error allocating arg");
            cleanup_argv(argv, i);
            return -1;
        }

        size_t k = 0;
        for (size_t j = 0; j < len; j++)
        {
            if (arg[j] == '{' && arg[j + 1] == '}')
            {
                strcpy(&rep[k], path);
                k += path_len;
                j++;
            }
            else
            {
                rep[k++] = arg[j];
            }
        }
        rep[k] = 0;
        argv[i] = rep;
    }

    int exit_status;
    pid_t pid = fork();
    switch (pid)
    {
    case -1:
        warn("eval_exec: fork");
        cleanup_argv(argv, ast->data.exec->argc);
        return -1;
    case 0:
        execvp(argv[0], argv);
        err(EXIT_FAILURE, "eval_exec: an error occured while executing '%s'",
            ast->data.exec->argv[0]);
    default:
        if (waitpid(pid, &exit_status, 0) == -1)
        {
            warn("eval_exec: waitpid failed");
            cleanup_argv(argv, ast->data.exec->argc);
            return -1;
        }
        if (!WIFEXITED(exit_status))
        {
            warnx("eval_exec: child process did not terminate normally");
            cleanup_argv(argv, ast->data.exec->argc);
            return -1;
        }
        break;
    }

    cleanup_argv(argv, ast->data.exec->argc);
    return exit_status == 0 ? 2 : -1;
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
    { .type = EXEC, .fun = eval_exec },
};

int eval(const char *path, const struct ast *ast)
{
    if (!ast)
        return 1;

    for (size_t i = 0; i < sizeof(alist_fun) / sizeof(alist_fun[0]); ++i)
        if (ast->type == alist_fun[i].type)
            return alist_fun[i].fun(path, ast);

    warnx("eval: invalid type '%d'", ast->type);
    return -1;
}
