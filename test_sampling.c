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

double SampleGamma_(double alpha)
{
    double t;
    double value = -1;
    while (value < 0)
    {
        t = 1.0 / (9 * alpha);
        value = alpha * pow(1 - t + sqrt(t) * SampleNormal(), 3);
    }
    return value;
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

    /*int n = 1000000000;

    printf("%ld\n", time(NULL));

    for (int i = 0; i < n; i++)
    {
        SampleGamma(a);
    }

    printf("%ld\n", time(NULL));

    for (int i = 0; i < n; i++)
    {
        SampleGamma_(a);
    }

    printf("%ld\n", time(NULL));*/
}