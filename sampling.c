#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

static uint64_t SEED;

struct Sample
{
    char result;
    struct Sample *next;
};

struct Sample* CreateSample(char result)
{
    struct Sample *sample = (struct Sample*)malloc(sizeof(struct Sample));
    sample->result = result;
    sample->next = NULL;
    return sample;
}

void SetSampling(int seed)
{
    SEED = (uint64_t)seed;
}

// (0, 1]かな？
double SampleUniform()
{
    SEED = SEED ^ (SEED << 7);
    SEED = SEED ^ (SEED >> 9);
    return (double)SEED / UINT64_MAX;
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

double SampleGamma(int alpha)
{
    double sample = 0;
    for(int i = 0; i < alpha; i++)
    {
        sample += SampleExponential();
    }
    return sample;
}

double SampleBeta(int a, int b)
{
    double gamma_a = SampleGamma(a);
    double gamma_b = SampleGamma(b);
    return gamma_a / (gamma_a + gamma_b);
}