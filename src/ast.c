#include "ast.h"

#include <err.h>
#include <stdlib.h>
#include <string.h>

struct alist assoc[] = {
    { .name = "-o", .type = OR },
    { .name = "-a", .type = AND },
    { .name = "-print", .type = PRINT },
    { .name = "-name", .type = NAME },
};

struct ast *ast_init(const char *name)
{
    struct ast *ast = malloc(sizeof(struct ast));
    if (!ast)
    {
        warn("ast_init: Memory failure");
        return NULL;
    }

    for (size_t i = 0; i < sizeof(assoc) / sizeof(assoc[0]); ++i)
    {
        if (strcmp(name, assoc[i].name) == 0)
        {
            ast->type = assoc[i].type;
            return ast;
        }
    }

    warnx("ast_init: Invalid name '%s'", name);
    free(ast);
    return NULL;
}

void ast_destroy(struct ast *ast)
{
    if (!ast)
        return;

    if (is_operator(ast))
    {
        ast_destroy(ast->data.children.right);
        ast_destroy(ast->data.children.left);
    }

    free(ast);
}

int is_operator(const struct ast *ast)
{
    return ast->type <= OPDELIM;
}

int is_action(const struct ast *ast)
{
    return ast->type > OPDELIM && ast->type <= ACTDELIM;
}

int is_test(const struct ast *ast)
{
    return ast->type > ACTDELIM;
}

// returns an integer indicating the result of the comparison, as follows:
// 0, if ast1 or ast2 has no precedence over the other;
// a positive value if ast1 precede over ast2;
// a negative value if ast2 precede over ast1;
int precedence(const struct ast *ast1, const struct ast *ast2)
{
    if (ast1->type == ast2->type)
        return 0;
    return (ast1->type > ast2->type) ? 1 : -1;
}
