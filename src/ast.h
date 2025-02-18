#ifndef AST_H
#define AST_H

#include <stddef.h>

#define OPDELIM 100
#define ACTDELIM 200

// Operators are <= OPDELIM and are
// sorted from lower to higher precedence
// Actions are > OPDELIM and <= ACTDELIM
// Tests are > ACTDELIM
enum type
{
    // Operators
    OR = 0,
    AND,
    LPAR,
    RPAR,
    NOT = OPDELIM,

    // Actions
    DELETE,
    EXEC,
    PRINT = ACTDELIM,

    // Tests
    NAME,
    TYPE,
    NEWER,
    PERM,
    USER,
    GROUP,
};

struct exec_info
{
    char **argv;
    size_t argc;
    char end;
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
        struct exec_info *exec;
    } data;
};

struct assoc_name
{
    enum type type;
    char *name;
};

struct ast *ast_init(const char *name);
void ast_destroy(struct ast *ast);
void cleanup_argv(char **argv, size_t argc);

int is_operator(const struct ast *ast);
int is_action(const struct ast *ast);
int is_test(const struct ast *ast);
int precedence(const struct ast *ast1, const struct ast *ast2);

#endif /* ! AST_H */
