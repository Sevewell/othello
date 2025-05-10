#include "engine.c"
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

extern double LEARNING_RATE;

int CountNode(struct Node *node, int *count) {

    if (node != NULL) {
        CountNode(node->next, count);
        CountNode(node->child, count);
        *count += 1;
    }

}

void ConvertToBinary(uint64_t stone) {
    if (stone > 1) ConvertToBinary(stone / 2);
    printf("%d", stone % 2);
}

void PrintNode(struct Node *node, int playout, int node_count)
{
    struct Node *child = node->child;
    uint64_t move = 0;
    uint64_t m = node->m;
    uint64_t y = node->y;
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
    printf("\"move\": '", move);
    ConvertToBinary(move);
    printf("', ");
    printf("\"m\": '");
    ConvertToBinary(m);
    printf("', ");
    printf("\"y\": '");
    ConvertToBinary(y);
    printf("', ");
    printf("\"rate\": %.4f, ", node->a / (node->a + node->b));
    node_count = node_count / 1000;
    printf("\"node\": '%dk'", node_count);
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
    uint64_t m = (uint64_t)strtoull(argv[1], NULL, 2);
    uint64_t y = (uint64_t)strtoull(argv[2], NULL, 2);
    unsigned int playout = atoi(argv[3]);
    int seed = atoi(argv[4]);
    LEARNING_RATE = strtod(argv[5], NULL);

    SetSampling(seed);
    SetZiggurat();
    SetupZigguratExpo();
    SetSamples();
    
    struct Node *node = CreateNode(m, y);
    Search(node, playout);

    return 0;
}
