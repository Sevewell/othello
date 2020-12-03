#include "engine.c"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern double LEARNING_RATE;

void PrintNode(struct Node *node)
{
    struct Node *child = node->child;
    unsigned long long move;
    unsigned long long m;
    unsigned long long y;
    double max_rate = 0;
    double rate;

    while (child)
    {
        rate = child->b / (child->a + child->b);
        if (rate > max_rate)
        {
            move = (node->m | node->y) ^ (child->m | child->y);
            m = child->y;
            y = child->m;
            max_rate = rate;
        }
        child = child->next;
    }

    printf("{ ");
    printf("\"m\": \"%llx\", ", m);
    printf("\"y\": \"%llx\", ", y);
    printf("\"move\": \"%llx\", ", move);
    printf("\"rate\": %lf", node->a / (node->a + node->b));
    printf(" }\n");
    fflush(stdout);

}

void Search(struct Node *node, unsigned int trial)
{
    char status[1024];

    char result;
    for (int i = 0; i < trial; i++)
    {
        result = 'n';
        PlayOut(node, &result);
        if (i % (trial / 100) == 0)
        {
            PrintNode(node);
        }
    }
}

int main(int argc, char *argv[])
{
    unsigned long long m = strtoull(argv[1], NULL, 2);
    unsigned long long y = strtoull(argv[2], NULL, 2);
    int seed = atoi(argv[3]);

    //int trial = atoi(getenv("TRIAL"));
    unsigned int trial = 500000;
    //LEARNING_RATE = strtod(getenv("LEARNING_RATE"), NULL);
    LEARNING_RATE = 0.99;

    SetSampling(seed);
    SetZiggurat();
    SetupZigguratExpo();

    struct Node *node = CreateNode(m, y);
    Search(node, trial);

    return 0;
}