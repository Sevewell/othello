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

void Test1DrawLotsNew()
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    unsigned long long movable = 0;
    double winrate = 1.0;

    struct Node* raffle = DrawLotsNew(node, &movable, &winrate);
    if (raffle != NULL)
    {
        printf("Error Test1: raffle should be NULL.\n");
    }
    if (node->child != NULL)
    {
        printf("Error Test1: wrong adding child.\n");
    }
    if (movable != 0)
    {
        printf("Error Test1: movable remained.\n");
    }
    if (winrate != 1.0)
    {
        printf("Error Test1: winrate is changed wrong.\n");
    }
}

void Test2DrawLotsNew()
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    node->child = CreateNode(68719476736, 34762915840); // Index is 0
    unsigned long long movable = 17729692106752; // Index 0 is removed.
    double winrate = 0.5;

    struct Node* raffle = DrawLotsNew(node, &movable, &winrate);
    if (raffle == NULL)
    {
        if (node->child->next != NULL)
        {
            printf("Error Test2: wrong adding child.\n");
        }
    }
    else
    {
        if (node->child->next == NULL)
        {
            printf("Error Test2: Child is not added.\n");
        }
        if (!((raffle->m == node->child->next->m) && (raffle->y == node->child->next->y)))
        {
            printf("Error Test2: Raffle is not added child.\n");
        }
        if ((raffle->m == 68719476736) && (raffle->y == 34762915840))
        {
            printf("Error Test2: Raffle is added existing child.\n");
            printf("%llu %llu\n", raffle->m, raffle->y);
        }
    }
    if (movable != 0)
    {
        printf("Error Test2: movable remained.\n");
    }
    if (winrate == 1.0)
    {
        printf("Error Test2: winrate is not writen.\n");
    }
}

void Test1PlayOut()
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    learning_rate = 0.9;

    PlayOut(node, 1);
    while (node != NULL)
    {
        printf("%llu %llu\n", node->m, node->y);
        printf("%lf %lf\n", node->a, node->b);
        printf("%c\n", node->result);
        node = node->child;
    }
}

void Test2PlayOut()
{
    struct Node* node = CreateNode(268435456, 61813844092928);
    learning_rate = 0.9;

    for (int i = 0; i < 100; i++)
    {
        PlayOut(node, 1);
    }

    if (node->result != 'l')
    {
        printf("Error Test2: wrong result.\n");
        printf("parent: %c\n", node->result);
        struct Node* child = node->child;
        while (child != NULL)
        {
            printf("child: %c\n", child->result);
            child = child->next;
        }
    }
}