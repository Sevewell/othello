#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "test_rule.c"

int main(int argc, char *argv[])
{
    printf("Testing!\n");
    Test_GetMovable_1();
    Test_GetReversable_1();
    Test_GetReversable_2();
    return 0;
}