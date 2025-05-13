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

void PrintNode(struct Node *node)
{
    struct Node *child = node->child;
    int node_count;
    printf("[");
    while (child)
    {
        printf("{");
        printf("'move': %"PRIu64", ", (child->m | child->y) ^ (node->m | node->y));
        printf("'m': %"PRIu64", ", child->y);
        printf("'y': %"PRIu64", ", child->m);
        printf("'a': %.4f, ", child->a);
        printf("'b': %.4f, ", child->b);
        node_count = 0;
        CountNode(child, &node_count);
        printf("'node': '%dk'", node_count / 1000);
        printf("}, ");
        child = child->next;
    }
    printf("]");
}

void Search(struct Node *node, unsigned int trial)
{
    clock_t clock_start = clock();
    clock_t clock_end;
    char result;
    for (int i = 1; i <= trial; i++)
    {
        result = 'n';
        PlayOut(node, &result);
    }
}

int main(int argc, char *argv[])
{
    uint64_t m = strtoull(argv[1], NULL, 10);
    uint64_t y = strtoull(argv[2], NULL, 10);
    unsigned int playout = atoi(argv[3]);
    int seed = atoi(argv[4]);
    LEARNING_RATE = strtod(argv[5], NULL);

    SetSampling(seed);
    SetZiggurat();
    SetupZigguratExpo();
    SetSamples();
    
    struct Node *node = CreateNode(m, y);
    Search(node, playout);

    PrintNode(node);

    return 0;
}
