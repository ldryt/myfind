#ifndef EVAL_H
#define EVAL_H

#include "ast.h"
#include "options.h"

struct assoc_fun
{
    enum type type;
    int (*fun)(const char *path, const struct ast *ast);
};

int eval(const char *path, const struct ast *ast);
int lsdir(const char *path, const struct ast *ast, const struct opt opt);

#endif /* ! EVAL_H */
