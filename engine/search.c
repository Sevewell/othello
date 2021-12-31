#include "engine.c"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

extern double LEARNING_RATE;

int CountNode(struct Node *node, int *count) {

    if (node != NULL) {
        CountNode(node->next, count);
        CountNode(node->child, count);
        *count += 1;
    }

}

void PrintNode(struct Node *node, int playout, int node_count)
{
    struct Node *child = node->child;
    unsigned long long move = 0;
    double max_rate = 0;
    double rate;

    while (child)
    {
        rate = child->b / (child->a + child->b);
        if (rate > max_rate)
        {
            move = (node->m | node->y) ^ (child->m | child->y);
            max_rate = rate;
        }
        child = child->next;
    }

    printf("{ ");
    printf("\"move\": \"%llx\", ", move);
    printf("\"rate\": %lf, ", node->a / (node->a + node->b));
    printf("\"node\": %d, ", node_count);
    printf("\"playout\": %d", playout);
    printf(" }\n");
    fflush(stdout);

}

void Search(struct Node *node, unsigned int trial)
{
    clock_t clock_start = clock();
    clock_t clock_end;
    int node_count;

    char result;
    for (int i = 1; i <= trial; i++)
    {
        result = 'n';
        PlayOut(node, &result);
    }

    node_count = 0;
    CountNode(node, &node_count);            
    PrintNode(node, trial, node_count);

}

int main(int argc, char *argv[])
{
    unsigned long long m = strtoull(argv[1], NULL, 2);
    unsigned long long y = strtoull(argv[2], NULL, 2);
    int seed = atoi(argv[3]);
    LEARNING_RATE = strtod(argv[4], NULL);

    //int trial = atoi(getenv("TRIAL"));
    unsigned int trial = 500000;

    SetSampling(seed);
    SetZiggurat();
    SetupZigguratExpo();

    struct Node *node = CreateNode(m, y);
    Search(node, trial);

    return 0;
}
