#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

struct Sample
{
    int result;
    struct Sample *next;
};

struct Position
{
    //uint64_t m;
    //uint64_t y;
    struct Sample *sample;
    int max_sample_length;
    int sample_length;
};

struct Sample* CreateSample(int result, struct Sample *next)
{
    struct Sample *sample = (struct Sample *)malloc(sizeof(struct Sample));
    sample->result = result;
    sample->next = next;
    return sample;
};

void AddSample(struct Position *node, char result)
{
    switch (result)
    {
        case 'w':
            result = 1;
            break;
        
        case 'l':
            result = 0;
            break;
    }

    struct Sample *new;
    if (node->sample == NULL)
    {
        new = CreateSample(result, NULL);
    }
    else
    {
        new = CreateSample(result, node->sample);
    }
    node->sample = new;
    node->sample_length++;        
}

int CompareUniform(const void *a, const void *b)
{
    float compare = *(float*)a - *(float*)b;
    if (compare > 0)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

float Sample(struct Position *node)
{
    int a = 1;
    int b = 1;

    int uniform_length = a + b;
    if (node->sample_length < node->max_sample_length)
    {
        uniform_length += node->sample_length;
    }
    else
    {
        uniform_length += node->max_sample_length;
    }
    float uniforms[uniform_length];

    struct Sample *current = node->sample;
    uniforms[0] = (float)rand() / RAND_MAX;
    uniforms[1] = (float)rand() / RAND_MAX;

    for (int i = 2; i < uniform_length; i++)
    {
        uniforms[i] = (float)rand() / RAND_MAX;
        if  (!current->result)
        {
            b++;
        }
        /*if (current->index == uniform_length)
        {
            current->next == NULL
        }*/
        current = current->next;
    }

    /*while(current != NULL)
    {
        // prevからのnext参照をクリアしないと
        printf("%d\n", current->result);
        struct Sample *tmp = current;
        current = current->next;
        free(tmp);
        printf("free\n");
    }*/

    qsort(uniforms, uniform_length, sizeof(float), CompareUniform);

    return uniforms[b]; // デフォルトでb=1なので-1しない
}

struct Position* CreatePosition(int max_sample_length)
{
    struct Position *node = (struct Position *)malloc(sizeof(struct Position));

    //struct Sample *tail = CreateSample(1, NULL);
    //node->sample = CreateSample(0, tail);
    node->sample_length = 0;
    //AddSample(node, 'w');
    //AddSample(node, 'l');
    node->max_sample_length = max_sample_length;

    return node;
}

void Playout(struct Position *node, float prob)
{
    float uniform = (float)rand() / RAND_MAX;
    char result;
    if (uniform < prob)
    {
        result = 'w';
    }
    else
    {
        result = 'l';
    }
    AddSample(node, result);
}

int main(int argc, char *argv[])
{
    printf("こんにちは\n");
    int seed = atoi(argv[1]);
    printf("Seedは%dです\n", seed);
    srand(seed);

    struct Position *hoge = CreatePosition(5);

    int trial = 10;
    float prob = 0.3;
    for (int i = 0; i < trial; i++)
    {
        Playout(hoge, prob);
        float sample = Sample(hoge);
        printf("%f\n", sample);
    }

    return 0;
}