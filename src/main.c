#include "ast_eval.h"
#include "errn.h"
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

int main(int argc, char **argv)
{
    int errn = PASS;

    struct opt opt = {
        .d = 0,
        .H = 0,
        .L = 0,
        .P = 1,
    };

    int opt_i = 1;
    while (opt_i < argc && get_opt(argv[opt_i], &opt))
        opt_i++;

    int expr_i = opt_i;
    while (expr_i < argc && !is_expr(argv[expr_i]))
        expr_i++;

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
