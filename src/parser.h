#ifndef PARSER_H
#define PARSER_H

#include "ast_eval.h"
#include "queue.h"

struct ast *parse(struct queue *queue);

#endif /* ! PARSER_H */
