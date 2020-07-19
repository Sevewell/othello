#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

double SampleUniform()
{
    double value;
    value = (double)rand() / RAND_MAX;
    assert(value >= 0);
    assert(value <= 1);
    return value;
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

double SampleBeta(double a, double b)
{
    double gamma1 = SampleGamma(a);
    double gamma2 = SampleGamma(b);
    double value = gamma1 / (gamma1 + gamma2);
    assert(value >= 0);
    assert(value <= 1);
    return value;
}