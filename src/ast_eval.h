#ifndef AST_H
#define AST_H

#define OPDELIM 100
#define ACTDELIM 200

#include "options.h"

// Operators are <= OPDELIM and are
// sorted from lower to higher precedence
// Actions are > OPDELIM and <= ACTDELIM
// Tests are > ACTDELIM
enum type
{
    // Operators
    OR = 0,
    AND,
    NOT = OPDELIM,

    // Actions
    PRINT = ACTDELIM,

    // Tests
    NAME,
    TYPE,
    NEWER,
    PERM,
    USER,
    GROUP,
};

struct ast
{
    enum type type;
    union
    {
        struct
        {
            struct ast *left;
            struct ast *right;
        } children;
        char *value;
    } data;
};

struct assoc
{
    char *name;
    enum type type;
    int (*fun)(const char *path, const struct ast *ast);
};

int lsdir(const char *path, const struct ast *ast, const struct opt opt,
          int is_arg);

int eval(const char *path, const struct ast *ast);
int eval_or(const char *path, const struct ast *ast);
int eval_and(const char *path, const struct ast *ast);
int eval_print(const char *path, const struct ast *ast);
int eval_name(const char *path, const struct ast *ast);
int eval_type(const char *path, const struct ast *ast);
int eval_newer(const char *path, const struct ast *ast);
int eval_perm(const char *path, const struct ast *ast);
int eval_user(const char *path, const struct ast *ast);
int eval_group(const char *path, const struct ast *ast);
int eval_not(const char *path, const struct ast *ast);

struct ast *ast_init(const char *name);
void ast_destroy(struct ast *ast);

int is_operator(const struct ast *ast);
int is_action(const struct ast *ast);
int is_test(const struct ast *ast);
int precedence(const struct ast *ast1, const struct ast *ast2);

#endif /* ! AST_H */
