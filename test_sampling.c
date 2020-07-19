#include "sampling.c"

int main(int argc, char *argv[])
{
    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    int seed = atoi(argv[3]);

    srand(seed);

    double sample;
    for (int i = 0; i < seed; i++)
    {
        sample = SampleBeta((double)a, (double)b);
        printf("%lf\n", sample);
    }
}