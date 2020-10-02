#include "engine.c"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern double LEARNING_RATE;

void PrintBinary(unsigned long long stone)
{
    unsigned long long mask = (unsigned long long)pow(2, 63);
    for (int i = 0; i < 64; i++)
    {
        if (stone & mask)
        {
            printf("1");
        }
        else
        {
            printf("0");
        }
        stone = stone << 1;
    }
    printf(", ");
}

void StreamInfo(struct Node *node, FILE *fp)
{
    struct Node *child = node->child;
    unsigned long long move;
    double winrate;
    while (child != NULL)
    {
        move = (child->m | child->y) ^ (node->m | node->y);
        winrate = child->b / (child->a + child->b);
        fprintf(fp, "%llu:%lf\n", move, winrate);
        child = child->next;
    }
}

void Search(struct Node *node, int trial)
{
    char result;
    FILE *fp;
    char filename[24];
    sprintf(filename, "./info/%d", getpid());
    fp = fopen(filename, "a");
    for (int i = 0; i < trial; i++)
    {
        result = 'n';
        PlayOut(node, &result);
        if (!(i % 1000))
        {
            StreamInfo(node, fp);
        }
    }
    fclose(fp);

    struct Node *child = node->child;
    while (child != NULL)
    {
        //printf("%llu, ", child->m);
        //printf("%llu, ", child->y);
        PrintBinary(child->m);
        PrintBinary(child->y);
        printf("%lf, ", child->a + child->b);
        printf("%lf\n", child->b / (child->a + child->b));
        child = child->next;
    }
}

int main(int argc, char *argv[])
{
    unsigned long long m = strtoull(argv[1], NULL, 2);
    unsigned long long y = strtoull(argv[2], NULL, 2);
    int seed = atoi(argv[3]);

    //int trial = atoi(getenv("TRIAL"));
    int trial = 1000000;
    //LEARNING_RATE = strtod(getenv("LEARNING_RATE"), NULL);
    LEARNING_RATE = 0.99;
    //int process = atoi(getenv("TREE"));
    int process = 4;

    SetSampling(seed);
    SetZiggurat();
    SetupZigguratExpo();

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