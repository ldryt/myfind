#include "lexer.h"

#include <err.h>
#include <stddef.h>

#include "ast_eval.h"

struct queue *lex(char **arg)
{
    struct queue *q = queue_init();

    int i = 0;
    while (arg[i])
    {
        struct ast *ast = ast_init(arg[i]);
        if (!ast)
        {
            warnx("lex: Unable to initialize ast");
            return NULL;
        }

        queue_push(q, ast);

        i++;
    }

    return q;
}
