#include "parser.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "errn.h"
#include "stack.h"

static int merge(struct ast *op, struct stack **cmd_stack)
{
    if (!*cmd_stack || (!(*cmd_stack)->next && op->type != NOT))
    {
        ast_destroy(op);
        warnx("merge: invalid expression (cmd_stack doesn't contain enough "
              "elements)");
        return FAIL;
    }

    op->data.children.right = stack_peek(*cmd_stack);
    *cmd_stack = stack_pop(*cmd_stack);

    if (op->type != NOT)
    {
        op->data.children.left = stack_peek(*cmd_stack);
        *cmd_stack = stack_pop(*cmd_stack);
    }

    *cmd_stack = stack_push(*cmd_stack, op);

    return PASS;
}

static void *abort_parsing(struct ast *elm, struct stack **cmd_stack,
                           struct stack **op_stack, char *msg)
{
    warnx("abort_parsing: %s", msg);
    ast_destroy(elm);
    stack_destroy(*cmd_stack);
    stack_destroy(*op_stack);
    return NULL;
}

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

struct ast *parse(struct queue *queue)
{
    struct ast *elm;
    int was_cmd = 0;
    struct stack *op_stack = NULL;
    struct stack *cmd_stack = NULL;

    if (!queue_seek(queue))
        return ast_init("-print");

    while ((elm = queue_pop(queue)))
    {
        if (!is_valid_value(elm->data.value, elm->type))
            return abort_parsing(elm, &cmd_stack, &op_stack,
                    "Invalid value");

        if (elm->type == LPAR)
        {
            op_stack = stack_push(op_stack, elm);
        }
        else if (elm->type == RPAR)
        {
            while (op_stack && !((stack_peek(op_stack))->type == LPAR))
            {
                struct ast *op = stack_peek(op_stack);
                op_stack = stack_pop(op_stack);

                if (merge(op, &cmd_stack) != PASS)
                    return abort_parsing(elm, &cmd_stack, &op_stack,
                                         "Unable to merge");
            }

            if (!op_stack || !((stack_peek(op_stack))->type == LPAR))
            {
                return abort_parsing(elm, &cmd_stack, &op_stack,
                                     "Mismatched parentheses");
            }

            struct ast *lpar = stack_peek(op_stack);
            op_stack = stack_pop(op_stack);
            ast_destroy(lpar);

            ast_destroy(elm);
        }
        else if (!is_operator(elm))
        {
            cmd_stack = stack_push(cmd_stack, elm);

            if (was_cmd)
                op_stack = stack_push(op_stack, ast_init("-a"));

            was_cmd = 1;
        }
        else
        {
            while (op_stack && !((stack_peek(op_stack)->type) == LPAR)
                   && precedence(elm, stack_peek(op_stack)) <= 0)
            {
                struct ast *op = stack_peek(op_stack);
                op_stack = stack_pop(op_stack);

                if (merge(op, &cmd_stack) != PASS)
                    return abort_parsing(elm, &cmd_stack, &op_stack,
                                         "Unable to merge");
            }

            op_stack = stack_push(op_stack, elm);

            was_cmd = 0;
        }
    }

    while (op_stack)
    {
        if ((stack_peek(op_stack))->type == LPAR)
        {
            return abort_parsing(NULL, &cmd_stack, &op_stack,
                                 "Mismatched parentheses");
        }

        struct ast *op = stack_peek(op_stack);
        op_stack = stack_pop(op_stack);

        if (merge(op, &cmd_stack) != PASS)
            return abort_parsing(NULL, &cmd_stack, &op_stack,
                                 "Unable to merge");
    }

    elm = stack_peek(cmd_stack);
    cmd_stack = stack_pop(cmd_stack);

    stack_destroy(cmd_stack);
    stack_destroy(op_stack);

    return elm;
}
