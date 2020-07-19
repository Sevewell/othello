#include "engine.c"

int TestUpdate()
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    int depth = 1;
    char result = 'w';
    Update(node, result, depth);
    assert(node->a == 2.0);
    assert(node->b == 1.0);
}

void Test1DrawLotsExisting()
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    unsigned long long movable = GetMovable(node->m, node->y);
    double winrate = 1.0;

    struct Node* raffle = DrawLotsExisting(node, &movable, &winrate);
    if (raffle != NULL)
    {
        printf("Error: Test1\n");
    }
    if (node->result != 'n')
    {
        printf("Error: Test1\n");
    }
    printf("%lf\n", winrate);
    printf("%llu %llu\n", movable, GetMovable(node->m, node->y));
}

void Test2DrawLotsExisting()
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    unsigned long long movable = GetMovable(node->m, node->y);
    double winrate = 1.0;
    node->child = CreateNode(68719476736, 34762915840); // index is 0

    struct Node* raffle = DrawLotsExisting(node, &movable, &winrate);
    if (raffle == NULL)
    {
        printf("Error Test2: choice is NULL\n");
    }
    else
    {
        if (!((raffle->m == 68719476736) && (raffle->y == 34762915840)))
        {
            printf("Error Test2: choice is wrong\n");
        }
    }
    if (winrate == 1.0)
    {
        printf("Error Test2: winrate is not writed\n");
    }
    if (node->result != 'n')
    {
        printf("Error Test2: node->result is changed\n");
    }
    printf("%lf\n", winrate);
    printf("%llu %llu\n", movable, GetMovable(node->m, node->y));
}

void Test3DrawLotsExisting()
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    unsigned long long movable = GetMovable(node->m, node->y);
    double winrate = 1.0;
    node->child = CreateNode(68719476736, 34762915840); // index is 0
    node->child->next = CreateNode(134217728, 17695533694976); // index is 3
    node->child->b = 99.0;

    struct Node* raffle = DrawLotsExisting(node, &movable, &winrate);
    if (raffle == NULL)
    {
        printf("Error Test3: choice is NULL\n");
    }
    else
    {
        if (!((raffle->m == 68719476736) && (raffle->y == 34762915840)))
        {
            printf("Error Test3: choice is wrong\n");
            printf("Error Test3: maybe miracle\n");
        }
    }
    if (winrate == 1.0)
    {
        printf("Error Test3: winrate is not writed\n");
    }
    if (node->result != 'n')
    {
        printf("Error Test3: node->result is changed\n");
    }
    printf("%lf\n", winrate);
    printf("%llu %llu\n", movable, GetMovable(node->m, node->y));
}

void TestDrawLotExisting()
{
    Test1DrawLotsExisting();
    Test2DrawLotsExisting();
    Test3DrawLotsExisting();
}

void TestPlayOut()
{
    struct Node* node = CreateNode(34628173824, 68853694464);

    for (int i = 0; i < 100; i++)
    {
        PlayOut(node, 1);
    }

    struct Node *child = node->child;

    while (child != NULL)
    {
        printf("a is %lf, ", child->a);
        printf("b is %lf\n", child->b);
        child = child->next;
    }
}

int main(int argc, char *argv[])
{
    int seed = atoi(argv[1]);
    srand(seed);

    TestUpdate();
    TestDrawLotExisting();
    TestPlayOut();

    return 0;
}