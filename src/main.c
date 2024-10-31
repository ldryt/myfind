#include "ast_eval.h"
#include "errn.h"
#include "lexer.h"
#include "parser.h"
#include "queue.h"

int main(int argc, char **argv)
{
    int errn = PASS;

    int expr_i = 1;
    while (expr_i < argc && argv[expr_i][0] != '-')
        expr_i++;

    if (expr_i >= argc)
    {
        if (argc == 1)
            return lsdir(".", NULL);

        for (int i = 1; i < expr_i; ++i)
        {
            if (lsdir(argv[i], NULL) != PASS)
                errn = FAIL;
        }
        return errn;
    }

    struct queue *q = lex(argv + expr_i);
    if (!q)
        return FAIL;

    struct ast *ast = parse(q);
    if (!ast)
        return FAIL;

    if (1 >= expr_i)
        if (lsdir(".", ast) != PASS)
            errn = FAIL;

    for (int i = 1; i < expr_i; ++i)
    {
        if (lsdir(argv[i], ast) != PASS)
            errn = FAIL;
    }

    queue_destroy(q);
    ast_destroy(ast);

    return errn;
}
