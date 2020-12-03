#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rule.c"

int main(int argc, char *argv[])
{
    unsigned long long m = strtoull(argv[1], NULL, 2);
    unsigned long long y = strtoull(argv[2], NULL, 2);
    int point = 63 - atoi(argv[3]);
    unsigned long long move = (unsigned long long)pow(2, point);

    unsigned long long reversable = GetReversable(m, y, move);
    if (reversable)
    {
        m = m | move | reversable;
        y = y ^ reversable;
    }

    printf("{ ");
    printf("\"m\": \"%llx\", ", m);
    printf("\"y\": \"%llx\"", y);
    printf(" }\n");

    return 0;
}