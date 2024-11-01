#include "lexer.h"

#include <err.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ast_eval.h"

static int is_valid_value_type(const char *value)
{
    if (strlen(value) != 1)
    {
        warnx("is_valid_value: value '%s' of type argument is not valid",
              value);
        return 0;
    }

    switch (value[0])
    {
    case 'f':
    /* FALLTHROUGH */
    case 'd':
    /* FALLTHROUGH */
    case 'c':
    /* FALLTHROUGH */
    case 'b':
    /* FALLTHROUGH */
    case 'p':
    /* FALLTHROUGH */
    case 'l':
    /* FALLTHROUGH */
    case 's':
        return 1;
    default:
        warnx("is_valid_value: value '%s' of type argument is not valid",
              value);
        return 0;
    }
}

static int is_valid_value_perm(const char *value)
{
    size_t len = strlen(value);
    switch (value[0])
    {
    case '-':
        return len > 1;
    case '/':
        return len > 1;
    default:
        return value[0] >= '0' && value[0] <= '9';
    }
}

static int is_valid_value(const char *value, enum type type)
{
    switch (type)
    {
    case TYPE:
        return is_valid_value_type(value);
    case PERM:
        return is_valid_value_perm(value);
    default:
        return 1;
    }
}

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

            // TODO: move to parser
            if (!is_valid_value(arg[i + 1], elm->type))
            {
                warnx("lex: invalid value '%s'", arg[i + 1]);
                ast_destroy(elm);
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
