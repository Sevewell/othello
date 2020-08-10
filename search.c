#include "engine.c"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern double LEARNING_RATE;

void Search(struct Node *node, int trial)
{
    char result;
    for (int i = 0; i < trial; i++)
    {
        result = 'n';
        PlayOut(node, &result);
    }
    struct Node *child = node->child;
    while (child != NULL)
    {
        printf("%llu, ", child->m);
        printf("%llu, ", child->y);
        printf("%lf, ", child->a + child->b);
        printf("%lf\n", child->b / (child->a + child->b));
        child = child->next;
    }
}

int main(int argc, char *argv[])
{
    unsigned long long m = strtoull(argv[1], NULL, 0);
    unsigned long long y = strtoull(argv[2], NULL, 0);
    int trial = atoi(argv[3]);
    LEARNING_RATE = strtod(argv[4], NULL);
    int process = atoi(argv[5]);
    int seed = atoi(argv[6]);

    SetSampling(seed);
    SetZiggurat();

    struct Node *node = CreateNode(m, y);
    pid_t pid[process];

    for (int i = 0; i < process; i++)
    {
        pid[i] = fork();
        if (pid[i] == -1)
        {
            exit(EXIT_FAILURE);
        }
        else if (pid[i] == 0)
        {
            Search(node, trial);
            exit(EXIT_SUCCESS);
        }
        else
        {
            SampleUniform(); // SEED更新
        }
    }

    for (int i = 0; i < process; i++)
    {
        wait(NULL);
    }

    return 0;
}