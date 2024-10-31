#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>

#include "lexer.h"

struct list
{
    struct ast *ast;
    struct list *next;
};

struct queue
{
    struct list *head;
    struct list *tail;
    size_t size;
};

struct queue *queue_init(void);
void queue_push(struct queue *q, struct ast *ast);
struct ast *queue_seek(struct queue *q);
struct ast *queue_pop(struct queue *q);
void queue_clear(struct queue *q);
void queue_destroy(struct queue *q);

#endif /* !QUEUE_H */
