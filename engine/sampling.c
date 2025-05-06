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
    srand(seed);
}

void SetZiggurat()
{
    ZIGGURAT_R = 3.6541528853610088;
    ZIGGURAT_V = 0.00492867323399;

    double X[256];
    X[255] = ZIGGURAT_R;
    F[255] = exp(-1 * ZIGGURAT_R * ZIGGURAT_R / 2);
    for (int i = 254; i > 0; i--)
    {
        X[i] = sqrt(-2 * log(F[i+1] + ZIGGURAT_V / X[i+1]));
        F[i] = exp(-1 * X[i] * X[i] / 2);
    }
    X[0] = 0;
    F[0] = 1;

    W[255] = ZIGGURAT_V * exp(ZIGGURAT_R * ZIGGURAT_R / 2) / pow(2, 64 - 8 - 1);
    K[255] = (uint64_t)(ZIGGURAT_R / W[255]);
    for (int i = 254; i > 0; i--)
    {
        W[i] = X[i+1] / pow(2, 64 - 8 - 1);
        K[i] = (uint64_t)(X[i] / W[i]);
    }
    W[0] = X[1] / pow(2, 64 - 8 - 1);
    K[0] = 0;
}

double ZIGGURAT_EXPO_R;
double ZIGGURAT_EXPO_V;
double W_EXPO[256];
uint64_t K_EXPO[256];
double F_EXPO[256];

void SetupZigguratExpo()
{
    ZIGGURAT_EXPO_R = 7.697117470131;
    ZIGGURAT_EXPO_V = 0.00394965982258;
    W_EXPO[255] = ZIGGURAT_EXPO_V * exp(ZIGGURAT_EXPO_R) / pow(2, 64 - 8);
    K_EXPO[255] = (uint64_t)(ZIGGURAT_EXPO_R / W_EXPO[255]);
    F_EXPO[255] = exp(-1 * ZIGGURAT_EXPO_R);

    double x = ZIGGURAT_EXPO_R;
    for (int i = 254; i > 0; i--)
    {
        W_EXPO[i] = x / pow(2, 64 - 8);
        x = -1 * log(F_EXPO[i + 1] + ZIGGURAT_EXPO_V / x);
        K_EXPO[i] = (uint64_t)(x / W_EXPO[i]);
        F_EXPO[i] = exp(-1 * x);
    }
    W_EXPO[0] = x / pow(2, 64 - 8);
    K_EXPO[0] = 0;
    F_EXPO[0] = 1;
}

uint64_t SampleUINT64()
{
    SEED = SEED ^ (SEED << 7);
    SEED = SEED ^ (SEED >> 9);
    return SEED;
}

double SampleUniform() // (0,1]?
{
    double uniform = (double)SampleUINT64() / UINT64_MAX;
    assert(uniform > 0);
    assert(uniform < 1);
    return uniform;
}

/*double SampleExponential()
{
    return -1 * log(SampleUniform());
}*/

double SampleExponential()
{
    uint64_t uniform_int;
    uint64_t i;
    uint64_t u_;
    double u_x;
    double f;
    double u__;
    while (1)
    {
        uniform_int = SampleUINT64();
        i = uniform_int & 255;
        u_ = uniform_int >> 8;
        if (u_ < K_EXPO[i])
        {
            return u_ * W_EXPO[i];
        }
        else if (i == 255)
        {
            return ZIGGURAT_EXPO_R - log(SampleUniform());
        }
        else
        {
            u_x = u_ * W_EXPO[i];
            f = exp(-1 * u_x);
            u__ = SampleUniform();
            if ((u__ * (F_EXPO[i] - F_EXPO[i + 1])) <= (f - F_EXPO[i + 1]))
            {
                return u_x;
            }
        }
    }
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
            double d = ZIGGURAT_R * ZIGGURAT_R;
            double u1;
            double u2;
            double x;
            while (1)
            {
                u1 = SampleUniform();
                u2 = SampleUniform();
                x = sqrt(d - 2 * log(u1));
                if (x * u2 <= ZIGGURAT_R)
                {
                    return x;
                }
            }
        }
        else
        {
            u_x = u__ * W[i];
            f = exp(-1 * u_x * u_x / 2);
            u___ = SampleUniform();
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
    if (alpha == 1.0)
    {
        return SampleExponential();
    }
    else
    {
        double d = alpha - 1.0 / 3.0;
        double c = 1.0 / sqrt(9.0 * d);
        double norm;
        double v;
        double w;
        double y;
        double u;
        while (1)
        {
            norm = SampleNormal();
            v = 1 + c * norm;
            if (v <= 0) continue;
            w = pow(v, 3);
            y = d * w;
            u = SampleUniform();
            if (u > (1 - 0.0331 * pow(norm, 4)))
            {
                if (norm * norm / 2 + d * log(w) - y + d < log(u)) continue;
            }
            return y;
        }        
    }
}

double SampleBeta(double a, double b)
{
    double gamma_a = SampleGamma(a);
    double gamma_b = SampleGamma(b);
    return gamma_a / (gamma_a + gamma_b);
}

#define MAX_A 32
#define MAX_B 32
#define NUM_SAMPLES 64

double SAMPLES[MAX_A*MAX_B][NUM_SAMPLES];

void SetHashTable()
{
    for (int a = 0; a < MAX_A; a++)
    {
        for (int b = 0; b < MAX_B; b++)
        {
            for (int n = 0; n < NUM_SAMPLES; n++)
            {
                SAMPLES[a*MAX_B+b][n] = SampleBeta(a+1, b+1);  // a+1, b+1にして0にならないようにする
            }
        }
    }
}

double SampleBetaFast(double a, double b, uint8_t index)
{
    if (a <= MAX_A && b <= MAX_B)
    {
        return SAMPLES[((int)a - 1)*MAX_B + ((int)b - 1)][index];
    }
    else
    {
        return SampleBeta(a, b);
    }
}