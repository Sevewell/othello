#include <stdint.h>
#include <inttypes.h>
#include <time.h>
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

void TestSampleNormal(int n)
{
    double sum = 0;

    double sample;
    for (int i = 0; i < n; i++)
    {
        sample = SampleNormal();
        sum += sample;
    }

    printf("Normal: E is %lf\n", sum / n);
}

void TestSampleGamma(double alpha, int n)
{
    double sum = 0;

    double sample;
    for (int i = 0; i < n; i++)
    {
        sample = SampleGamma(alpha);
        assert(sample >= 0);
        sum += sample;
    }

    printf("Gamma: E is %lf\n", sum / n);
}

void TestSampleBeta(double a, double b, int n)
{
    double sum = 0;

    double sample;
    double a_gamma;
    double b_gamma;
    for (int i = 0; i < n; i++)
    {
        sample = SampleBeta(a, b);
        assert(sample >= 0);
        assert(sample <= 1);
        sum += sample;
    }

    printf("Beta: E is %lf\n", sum / n);
}

double SampleNormal_()
{
    double sum = 0;
    for (int i = 0; i < 12; i++)
    {
        sum += SampleUniform();
    }
    return sum - 6;
}

int main(int argc, char *argv[])
{
    double a = strtod(argv[1], NULL);
    double b = strtod(argv[2], NULL);
    int seed = atoi(argv[3]);

    SetSampling(seed);
    SetZiggurat();

    TestSampleUniform(seed);
    TestSampleExponential(seed);
    TestSampleNormal(seed);
    TestSampleGamma(a, seed);
    TestSampleGamma(b, seed);
    TestSampleBeta(a, b, seed);

    int n = 100000000;

    printf("%ld\n", time(NULL));

    for (int i = 0; i < n; i++)
    {
        SampleNormal_();
    }

    printf("%ld\n", time(NULL));

    for (int i = 0; i < n; i++)
    {
        SampleNormal();
    }

    printf("%ld\n", time(NULL));
}