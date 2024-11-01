#include "options.h"

#include <string.h>

int get_opt(char *str, struct opt *opt)
{
    if (strlen(str) != 2 || str[0] != '-')
        return 0;

    switch (str[1])
    {
    case 'd':
        opt->d = 1;
        break;
    case 'H':
        opt->H = 1;
        opt->L = 0;
        break;
    case 'L':
        opt->L = 1;
        opt->H = 0;
        opt->P = 0;
        break;
    case 'P':
        opt->P = 1;
        opt->L = 0;
        opt->H = 0;
        break;
    default:
        return 0;
    }

    return 1;
}
