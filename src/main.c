#include "ast.h"
#include "errn.h"
#include "eval.h"
#include "lexer.h"
#include "options.h"
#include "parser.h"
#include "queue.h"

static int is_expr(char *str)
{
    switch (str[0])
    {
    case '-':
    /* FALLTHROUGH */
    case '!':
    /* FALLTHROUGH */
    case '(':
    /* FALLTHROUGH */
    case ')':
        return 1;
    default:
        return 0;
    }
}

int find_options_idx(int argc, char **argv, struct opt *opt)
{
    int opt_i = 1;
    while (opt_i < argc && get_opt(argv[opt_i], opt))
        opt_i++;
    return opt_i;
}

int find_expressions_idx(int argc, char **argv, int start_index)
{
    int expr_i = start_index;
    while (expr_i < argc && !is_expr(argv[expr_i]))
        expr_i++;
    return expr_i;
}

int main(int argc, char **argv)
{
    int errn = PASS;

    struct opt opt = { .d = 0, .H = 0, .L = 0, .P = 1 };

    int opt_i = find_options_idx(argc, argv, &opt);
    int expr_i = find_expressions_idx(argc, argv, opt_i);

    if (expr_i >= argc)
    {
        if (expr_i == opt_i)
            return lsdir(".", NULL, opt, 1);

        for (int i = opt_i; i < expr_i; ++i)
        {
            if (lsdir(argv[i], NULL, opt, 1) != PASS)
                errn = FAIL;
        }
        return errn;
    }

    struct queue *q = lex(argv + expr_i);
    if (!q)
        return FAIL;

    struct ast *ast = parse(q);
    if (!ast)
    {
        queue_destroy(q);
        return FAIL;
    }

    if (opt_i >= expr_i)
        if (lsdir(".", ast, opt, 1) != PASS)
            errn = FAIL;

    for (int i = opt_i; i < expr_i; ++i)
    {
        if (lsdir(argv[i], ast, opt, 1) != PASS)
            errn = FAIL;
    }

    queue_destroy(q);
    ast_destroy(ast);

    return errn;
}
