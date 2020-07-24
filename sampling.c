#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

static uint64_t SEED;
static double r = 3.6541528853610088;
static double v = 0.00492867323399;

void SetSampling(int seed)
{
    SEED = (uint64_t)seed;
    return;
}

// 性質上0にはならない？
uint64_t SampleInt64()
{
    SEED = SEED ^ (SEED << 7);
    SEED = SEED ^ (SEED >> 9);
    return SEED;
}

// (0, 1]かな？
double SampleUniform()
{
    return (double)SampleInt64() / UINT64_MAX;
}

double SampleExponential()
{
    return -1 * log(SampleUniform());
}

double SampleNormal()
{
    double sum = 0;
    for (int i = 0; i < 12; i++)
    {
        sum += SampleUniform();
    }
    return sum - 6.0;
}

double SampleGamma(double alpha)
{
    double c1 = alpha - 1.0 / 3.0;
    double c2 = 1.0 / sqrt(9.0 * c1);
    double norm;
    double v;
    double u;
    while (1)
    {
        norm = SampleNormal();
        if (c2 * norm <= -1.0) continue;
        v = pow(1.0 + c2 * norm, 3.0);
        u = SampleUniform();
        if (u < 1.0 - 0.331 * pow(norm, 4.0)) return c1 * v;
        if (log(u) < 0.5 * pow(norm, 2.0) + c1 * (1.0 - v + log(v))) return c1 * v;
    }
}

double SampleBeta(double a_sum, double b_sum)
{
    double a = SampleExponential() + a_sum;
    double b = SampleExponential() + b_sum;
    return a / (a + b);
}