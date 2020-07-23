#include "engine.c"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int MakeChildren(struct Node *node)
{
    unsigned long long movable = GetMovable(node->m, node->y);
    int count = 0;

    if (movable)
    {
        unsigned long long move;
        unsigned long long reversable;
        struct Node *child;
        while (movable)
        {
            count++;
            move = movable ^ (movable & (movable - 1));
            reversable = GetReversable(node->m, node->y, move);
            child = CreateNode(node->y ^ reversable, node->m | move | reversable);
            AddChild(node, child);
            movable &= (movable - 1);
        }
    }
    else if (GetMovable(node->y, node->m))
    {
        node->child = CreateNode(node->y, node->m);
        count = 1;
    }

    return count;
}

void SearchChild(struct Node *child, int *count_process, int max_process, int trial, double learning_rate)
{
    if (child == NULL)
    {
        while(*count_process != 0)
        {
            wait(NULL);
            *count_process -= 1;
        }
        return;
    }

    pid_t pid;
    pid = fork();

    if (pid == -1)
    {
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        for (int j = 0; j < trial; j++)
        {
            double value;
            PlayOut(child, &value, learning_rate);
        }

        printf("%llu, ", child->m);
        printf("%llu, ", child->y);
        printf("%lf, ", child->a);
        printf("%lf, ", child->b);
        printf("%lf\n", child->a / (child->a + child->b));

        exit(EXIT_SUCCESS);
    }
    else
    {
        *count_process += 1;
        if (*count_process == max_process)
        {
            wait(NULL);
            *count_process -= 1;
        }
        SearchChild(child->next, count_process, max_process, trial, learning_rate);
        return;
    }
}

int main(int argc, char *argv[])
{
    unsigned long long m = strtoull(argv[1], NULL, 0);
    unsigned long long y = strtoull(argv[2], NULL, 0);
    int trial = atoi(argv[3]);
    double learning_rate = strtod(argv[4], NULL);
    int process = atoi(argv[5]);
    int seed = atoi(argv[6]);

    srand(seed);

    struct Node *node = CreateNode(m, y);
    int count_children = MakeChildren(node);
    int trial_child;
    if (count_children == 1) // カウント１だと探索時間がかかってしまうので
    {
        trial_child = trial / 2;
    }
    else
    {
        trial_child = trial / count_children;    
    }

    struct Node *child = node->child;
    int count_process = 0;
    SearchChild(child, &count_process, process, trial_child, learning_rate);

    return 0;
}