#include <stdint.h>
#include <inttypes.h>
#include "sampling.c"

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

void TestSampleExponential(int n)
{
    double sum = 0;

    double sample;
    for (int i = 0; i < n; i++)
    {
        sample = SampleExponential();
        assert(sample >= 0);
        sum += sample;
    }

    printf("Exponential: E is %lf\n", sum / n);
}

void TestSampleBeta(int a, int b, int n)
{
    double sum = 0;

    double sample;
    double a_gamma;
    double b_gamma;
    for (int i = 0; i < n; i++)
    {
        a_gamma = SampleGamma(a - 1);
        b_gamma = SampleGamma(b - 1);
        sample = SampleBeta(a_gamma, b_gamma);
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

    TestSampleUniform(seed);
    TestSampleExponential(seed);
    TestSampleBeta(a, b, seed);
}