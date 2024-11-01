#include "lexer.h"

#include <err.h>
#include <stddef.h>
#include <stdlib.h>

#include "ast_eval.h"

struct queue *lex(char **arg)
{
    struct queue *q = queue_init();

    int i = 0;
    while (arg[i])
    {
        struct ast *elm = ast_init(arg[i]);
        if (!elm)
        {
            warnx("lex: Unable to initialize elm");
            return NULL;
        }

        if (is_test(elm))
        {
            if (!arg[i + 1])
            {
                ast_destroy(elm);
                warnx("lex: No value after test argument");
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
