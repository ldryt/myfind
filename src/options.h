#ifndef OPTIONS_H
#define OPTIONS_H

struct opt
{
    int d;
    int H;
    int L;
    int P;
    int is_arg;
    int eval_print;
};

int get_opt(char *str, struct opt *opt);

#endif /* ! OPTIONS_H */
