#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "queue.h"

struct ast *parse(struct queue *queue, int *print);

#endif /* ! PARSER_H */
