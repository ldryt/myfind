#ifndef STACK_H
#define STACK_H

#include "lexer.h"

struct stack
{
    struct ast *ast;
    struct stack *next;
};

struct stack *stack_push(struct stack *s, struct ast *ast);
struct stack *stack_pop(struct stack *s);
struct ast *stack_peek(struct stack *s);
void stack_destroy(struct stack *s);

#endif /* !STACK_H */
