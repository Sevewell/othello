#include "engine.c"

int main(int argc, char *argv[])
{
    unsigned long long m = strtoull(argv[1], NULL, 0);
    unsigned long long y = strtoull(argv[2], NULL, 0);
    int trial = atoi(argv[3]);
    double learning_rate = strtod(argv[4], NULL);
    int seed = atoi(argv[5]);

    srand(seed);

    struct Node* node = CreateNode(m, y);

    for (int j = 0; j < trial; j++)
    {  
        //printf("\r%d/%d", j, trial);
        double value;
        PlayOut(node, &value, learning_rate);
    }
    //printf("\r\n");

    struct Node *child = node->child;
    while (child != NULL)
    {
        printf("%llu, ", child->m);
        printf("%llu, ", child->y);
        printf("%lf, ", child->a);
        printf("%lf, ", child->b);
        printf("%lf\n", child->a / (child->a + child->b));
        child = child->next;
    }

    return 0;
}