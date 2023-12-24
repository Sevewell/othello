#include "rule.c"

void Test_GetMovable_1()
{
    uint64_t m = 567382630203392;
    uint64_t y = 72057662891638784;
    uint64_t expectation = 17729692631168;
    uint64_t output = GetMovable(m, y);
    if (output == expectation)
    {
        printf("Correct!\n");
    }
    else
    {
        printf("Wrong!: %" PRIu64, output);
        printf("\n");
    }
}

void Test_GetReversable_1()
{
    uint64_t m = 585467951558295808;
    uint64_t y = 2278188294604808;
    uint64_t move = 34359738368;
    uint64_t expectation = 2278188159860736;
    uint64_t output = GetReversable(m, y, move);
    if (output == expectation)
    {
        printf("Correct!\n");
    }
    else
    {
        printf("Wrong!: %" PRIu64, output);
        printf("\n");
    }
}

void Test_GetReversable_2()
{
    uint64_t m = 2260595906707464;
    uint64_t y = 576460786663688192;
    uint64_t move = 134217728;
    uint64_t expectation = 34360264704;
    uint64_t output = GetReversable(m, y, move);
    if (output == expectation)
    {
        printf("Correct!\n");
    }
    else
    {
        printf("Wrong!: %" PRIu64, output);
        printf("\n");
    }
}