#include "engine.c"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern double LEARNING_RATE;

void PrintNode(struct Node *node)
{
    struct Node *child = node->child;

    printf("[ ");

    while (1)
    {
        printf("{ ");
        printf("\"m\": \"%llx\", ", child->y);
        printf("\"y\": \"%llx\", ", child->m);
        printf("\"move\": \"%llx\", ", (node->m | node->y) ^ (child->m | child->y));
        printf("\"rate\": %lf ", child->b / (child->a + child->b));
        printf(" }");

        child = child->next;

        if (child)
        {
            printf(", ");
        }
        else
        {
            printf(" ]\n");
            fflush(stdout);
            break;
        }
    }
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
    unsigned int trial = 700000;
    //LEARNING_RATE = strtod(getenv("LEARNING_RATE"), NULL);
    LEARNING_RATE = 0.99;

    SetSampling(seed);
    SetZiggurat();
    SetupZigguratExpo();

    struct Node *node = CreateNode(m, y);
    Search(node, trial);

    return 0;
}