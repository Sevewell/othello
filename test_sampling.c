#include <stdint.h>
#include <inttypes.h>
#include "sampling.c"

void TestSampleInt64(int n)
{
    uint64_t sample;
    for (int i = 0; i < n; i++)
    {
        sample = SampleInt64();
        //printf("Int64: %" PRIu64 "\n", sample);
        assert(sample >= 0);
        assert(sample <= UINT64_MAX);
    }
}

void TestSampleUniform(int n)
{
    double sum = 0;

    double sample;
    for (int i = 0; i < n; i++)
    {
        sample = SampleUniform();
        assert(sample >= 0);
        assert(sample <= 1);
        sum += sample;
    }

    printf("Uniform: E is %lf\n", sum / n);
}

void TestSampleBeta(int a, int b, int n)
{
    double sum = 0;

    double sample;
    for (int i = 0; i < n; i++)
    {
        sample = SampleBeta((double)a, (double)b);
        assert(sample >= 0);
        assert(sample <= 1);
        sum += sample;
    }

    printf("Beta: E is %lf\n", sum / n);
}

int main(int argc, char *argv[])
{
    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    int seed = atoi(argv[3]);

    SetSampling(seed);

    TestSampleInt64(seed);
    TestSampleUniform(seed);
    TestSampleBeta(a, b, seed);
}