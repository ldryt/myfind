#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

struct queue *queue_init(void)
{
    struct queue *q = malloc(sizeof(struct queue));
    if (!q)
        return NULL;

    q->tail = NULL;
    q->head = NULL;
    q->size = 0;

    return q;
}

void queue_clear(struct queue *q)
{
    struct list *p = q->head;
    while (p)
    {
        struct list *tmp = p;
        p = p->next;
        free(tmp->ast);
        free(tmp);
    }
    q->size = 0;

    q->head = NULL;
    q->tail = NULL;
}

void queue_destroy(struct queue *q)
{
    queue_clear(q);
    free(q);
    q = NULL;
}

void queue_push(struct queue *q, struct ast *ast)
{
    struct list *new = malloc(sizeof(struct list));
    if (!new)
        return;

    new->ast = ast;
    new->next = NULL;

    if (!q->head)
        q->head = new;

    if (q->tail)
        q->tail->next = new;
    q->tail = new;

    q->size++;
}

struct ast *queue_seek(struct queue *q)
{
    if (!q->head)
        return NULL;

    return q->head->ast;
}

struct ast *queue_pop(struct queue *q)
{
    if (!q->head)
        return NULL;

    struct ast *ast = q->head->ast;

    struct list *tmp = q->head;
    q->head = q->head->next;
    q->size--;
    free(tmp);

    return ast;
}
