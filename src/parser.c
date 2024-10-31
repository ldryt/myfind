#include "parser.h"

#include <stddef.h>

#include "errn.h"
#include "stack.h"

static int merge(struct ast *op, struct stack **cmd_stack)
{
    if (!*cmd_stack || !(*cmd_stack)->next)
    {
        warnx("merge: invalid expression (cmd_stack doesn't contain enough "
              "elements)");
        return FAIL;
    }

    op->data.children.right = stack_peek(*cmd_stack);
    *cmd_stack = stack_pop(*cmd_stack);

    op->data.children.left = stack_peek(*cmd_stack);
    *cmd_stack = stack_pop(*cmd_stack);

    *cmd_stack = stack_push(*cmd_stack, op);

    return PASS;
}

static void *abort_parsing(struct ast *elm, struct stack **cmd_stack,
                           struct stack **op_stack, char *msg)
{
    warn("abort_parsing: %s", msg);
    ast_destroy(elm);
    stack_destroy(*cmd_stack);
    stack_destroy(*op_stack);
    return NULL;
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
        if (!is_operator(elm))
        {
            cmd_stack = stack_push(cmd_stack, elm);

            if (was_cmd)
                op_stack = stack_push(op_stack, ast_init("-a"));

            was_cmd = 1;
        }
        else
        {
            struct ast *op;
            if (op_stack && precedence(elm, (op = stack_peek(op_stack))) < 0)
            {
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
        struct ast *op = stack_peek(op_stack);
        op_stack = stack_pop(op_stack);
        if (merge(op, &cmd_stack) != PASS)
            return abort_parsing(elm, &cmd_stack, &op_stack, "Unable to merge");
    }

    elm = stack_peek(cmd_stack);
    cmd_stack = stack_pop(cmd_stack);

    stack_destroy(cmd_stack);
    stack_destroy(op_stack);

    return elm;
}
