#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

static uint64_t SEED;

double ZIGGURAT_R;
double ZIGGURAT_V;

double W[256];
uint64_t K[256];
double F[256];

void SetSampling(int seed)
{
    SEED = (uint64_t)seed;
    srandom(seed);
}

void SetZiggurat()
{
    ZIGGURAT_R = 3.6541528853610088;
    ZIGGURAT_V = 0.00492867323399;

    double X[256];
    X[255] = ZIGGURAT_R;
    F[255] = exp(-1 * pow(ZIGGURAT_R, 2.0) / 2);
    for (int i = 254; i > 0; i--)
    {
        X[i] = sqrt(-2 * log(F[i+1] + ZIGGURAT_V / X[i+1]));
        F[i] = exp(-1 * pow(X[i], 2.0) / 2);
    }
    X[0] = 0;
    F[0] = 1;

    W[255] = ZIGGURAT_V * exp(pow(ZIGGURAT_R, 2.0) / 2) / pow(2, 64 - 8 - 1);
    K[255] = (uint64_t)(ZIGGURAT_R / W[255]);
    for (int i = 254; i > 0; i--)
    {
        W[i] = X[i+1] / pow(2, 64 - 8 - 1);
        K[i] = (uint64_t)(X[i] / W[i]);
    }
    W[0] = X[1] / pow(2, 64 - 8 - 1);
    K[0] = 0;
}

uint64_t SampleUINT64()
{
    SEED = SEED ^ (SEED << 7);
    SEED = SEED ^ (SEED >> 9);
    return SEED;
}

double SampleUniform() // (0,1]
{
    return (double)SampleUINT64() / UINT64_MAX;
}

double SampleExponential()
{
    return -1 * log(SampleUniform());
}

double SampleNormal()
{
    uint64_t u;
    uint64_t i;
    uint64_t u_;
    uint8_t sign;
    uint64_t u__;
    double u_x;
    double f;
    double d;
    double u___;
    while (1)
    {
        u = SampleUINT64();
        i = u & 255;
        u_ = u >> 8;
        sign = u_ & 1;
        u__ = u_ >> 1;
        if (u__ < K[i])
        {
            u_x = u__ * W[i];
            if (sign)
            {
                u_x *= -1;
            }
            return u_x;
        }
        else if (i == 255)
        {
            double d = pow(ZIGGURAT_R, 2);
            double u1;
            double u2;
            double x;
            while (1)
            {
                u1 = (double)random() / ((double)RAND_MAX + 1);
                u2 = (double)random() / (double)RAND_MAX;
                x = sqrt(d - 2 * log(1 - u1));
                if (x * u2 <= ZIGGURAT_R)
                {
                    return x;
                }
            }
        }
        else
        {
            u_x = u__ * W[i];
            f = exp(-1 * pow(u_x, 2) / 2);
            u___ = (double)random() / ((double)RAND_MAX + 1);
            if ((u___ * (F[i] - F[i + 1])) <= (f - F[i + 1]))
            {
                if (sign)
                {
                    u_x *= -1;
                }
                return u_x;
            }
        }
    }
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
    double gamma_a = SampleGamma(a);
    double gamma_b = SampleGamma(b);
    return gamma_a / (gamma_a + gamma_b);
}