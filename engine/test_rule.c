#include "rule.c"

void Test_GetMovable_1()
{
    uint64_t m = 567382630203392;
    uint64_t y = 72057662891638784;
    uint64_t expectation = 17729692631168;
    uint64_t output = GetMovable(m, y);
    if (output == expectation)
    {
        printf("Correct!");
    }
    else
    {
        printf("Wrong!: %" PRIu64, output);
    }
}

void TEst_GetReversable_1()
{
    uint64_t m = 4398046511104;
    uint64_t y = 562949953421312;
    uint64_t move = 72057594037927936;
    uint64_t expectation = 562949953421312;
    uint64_t output = GetReversable(m, y, move);
    if (output == expectation)
    {
        printf("Correct!");
    }
    else
    {
        printf("Wrong!: %" PRIu64, output);
    }
}