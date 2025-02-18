#define _POSIX_C_SOURCE 200809L // strdup

#include "lexer.h"

#include <err.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

static int lex_exec(char **arg, struct ast *elm, size_t *i)
{
    struct exec_info *exec = malloc(sizeof(struct exec_info));
    if (!exec)
    {
        warnx("lex: unable to initialize exec structure");
        return -1;
    }
    exec->argv = NULL;
    exec->argc = 0;
    exec->end = 0;

    (*i)++;
    while (arg[*i] && strcmp(arg[*i], ";") != 0 && strcmp(arg[*i], "+") != 0)
    {
        exec->argv = realloc(exec->argv, (exec->argc + 1) * sizeof(char *));
        exec->argv[exec->argc] = strdup(arg[*i]);
        exec->argc++;
        (*i)++;
    }
    if (!arg[*i])
    {
        cleanup_argv(exec->argv, exec->argc);
        free(exec);
        warnx("lex: expected ';' at the end of '-exec' expression");
        return -1;
    }

    exec->end = arg[*i][0];

    elm->data.exec = exec;

    return 0;
}

struct queue *lex(char **arg)
{
    struct queue *q = queue_init();

    size_t i = 0;
    while (arg[i])
    {
        struct ast *elm = ast_init(arg[i]);
        if (!elm)
        {
            queue_destroy(q);
            warnx("lex: unable to initialize ast element");
            return NULL;
        }

        if (elm->type == EXEC)
        {
            if (lex_exec(arg, elm, &i) < 0)
            {
                queue_destroy(q);
                ast_destroy(elm);
                return NULL;
            }
        }
        else if (is_test(elm))
        {
            if (!arg[i + 1])
            {
                queue_destroy(q);
                ast_destroy(elm);
                warnx("lex: no value after test argument");
                return NULL;
            }

            elm->data.value = arg[i + 1];
            i++;
        }

        queue_push(q, elm);
        i++;
    }

    return q;
}
