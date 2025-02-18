#include "ast.h"

#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "errn.h"

struct assoc_name alist_name[] = {
    { .type = AND, .name = "-a" },         { .type = OR, .name = "-o" },
    { .type = PRINT, .name = "-print" },   { .type = NAME, .name = "-name" },
    { .type = TYPE, .name = "-type" },     { .type = NEWER, .name = "-newer" },
    { .type = PERM, .name = "-perm" },     { .type = USER, .name = "-user" },
    { .type = GROUP, .name = "-group" },   { .type = NOT, .name = "!" },
    { .type = LPAR, .name = "(" },         { .type = RPAR, .name = ")" },
    { .type = DELETE, .name = "-delete" }, { .type = EXEC, .name = "-exec" },
};

struct ast *ast_init(const char *name)
{
    struct ast *ast = malloc(sizeof(struct ast));
    if (!ast)
    {
        warn("ast_init: unable to allocate ast structure");
        return NULL;
    }

    ast->data.value = NULL;
    ast->data.children.left = NULL;
    ast->data.children.right = NULL;

    for (size_t i = 0; i < sizeof(alist_name) / sizeof(alist_name[0]); ++i)
    {
        if (strcmp(name, alist_name[i].name) == 0)
        {
            ast->type = alist_name[i].type;
            return ast;
        }
    }

    warnx("ast_init: not a valid name: '%s'", name);
    free(ast);
    return NULL;
}

void cleanup_argv(char **argv, size_t argc)
{
    for (size_t i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);
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
    else if (is_action(ast))
    {
        if (ast->type == EXEC && ast->data.exec)
        {
            cleanup_argv(ast->data.exec->argv, ast->data.exec->argc);
            free(ast->data.exec);
        }
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
