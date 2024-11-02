#ifndef EVAL_H
#define EVAL_H

#include "options.h"
#include "ast.h"

struct assoc_fun
{
    enum type type;
    int (*fun)(const char *path, const struct ast *ast);
};

int eval(const char *path, const struct ast *ast);
int lsdir(const char *path, const struct ast *ast, const struct opt opt,
          int is_arg);

#endif /* ! EVAL_H */
