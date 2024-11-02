#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "queue.h"
#include "options.h"

struct ast *parse(struct queue *queue, struct opt *opt);

#endif /* ! PARSER_H */
