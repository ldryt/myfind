#include "stack.h"

#include <stdlib.h>

struct stack *stack_push(struct stack *s, struct ast *ast)
{
    struct stack *p = malloc(sizeof(struct stack));
    if (!p)
        return s;

    p->ast = ast;
    p->next = s;

    return p;
}

struct stack *stack_pop(struct stack *s)
{
    if (s == NULL)
        return NULL;
    struct stack *p = s->next;
    free(s);
    return p;
}

struct ast *stack_peek(struct stack *s)
{
    return s->ast;
}

void stack_destroy(struct stack *s)
{
    struct ast *tmp;
    while (s)
    {
        tmp = stack_peek(s);
        s = stack_pop(s);
        ast_destroy(tmp);
    }
}
