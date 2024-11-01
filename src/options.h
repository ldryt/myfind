#ifndef OPTIONS_H
#define OPTIONS_H

struct opt
{
    int d;
    int H;
    int L;
    int P;
};

int get_opt(char *str, struct opt *opt);

#endif /* ! OPTIONS_H */
