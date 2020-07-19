#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <x86intrin.h>
#include <math.h>
#include <assert.h>
#include "rule.c"
#include "sampling.c"

struct Node
{
    unsigned long long m;
    unsigned long long y;
    double a;
    double b;
    char result;
    struct Node *child;
    struct Node *next;
};

struct Node* CreateNode(unsigned long long m, unsigned long long y)
{
    struct Node* node;
    node = (struct Node*)malloc(sizeof(struct Node));

    node->m = m;
    node->y = y;
    node->a = 1;
    node->b = 1;
    node->child = NULL;
    node->next = NULL;

    return node;
}

void Free(struct Node* node)
{
    if (node == NULL) return;
    Free(node->child);
    Free(node->next);
    free(node);
    node = NULL;
}

void AddChild(struct Node *node, struct Node* child)
{
    if (node->child == NULL)
    {
        node->child = child;
    }
    else
    {
        struct Node* last = node->child;
        while (last->next != NULL)
        {
            last = last->next;
        }
        last->next = child;
    }
}

struct Node* DrawLotsExisting(struct Node* node, unsigned long long *movable, double *winrate)
{
    struct Node* choice = NULL;
    struct Node* child = node->child;
    double sample;

    while (child != NULL)
    {
        sample = SampleBeta(child->a, child->b);

        if (sample <= *winrate)
        {
            choice = child;
            *winrate = sample;
        }

        *movable ^= (*movable & (child->m | child->y));
        child = child->next;
    }

    return choice;
}

struct Node* DrawLotsNew(struct Node* node, unsigned long long *movable, double *winrate)
{
    struct Node* raffle = NULL;
    double sample;
    unsigned long long move;
    int flag = 0;

    while (*movable)
    {
        sample = SampleUniform();

        if (sample <= *winrate)
        {
            flag = 1;
            move = *movable ^ (*movable & (*movable - 1));
            *winrate = sample;
        }

        *movable &= (*movable - 1);
    }

    if (flag)
    {
        unsigned long long reversable = GetReversable(node->m, node->y, move);
        raffle = CreateNode(node->y ^ reversable, node->m | move | reversable);
        AddChild(node, raffle);
    }

    return raffle;
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

struct Node* Move(struct Node* node, unsigned long long movable)
{

    double winrate = 1.0;

    struct Node* raffle_existing = DrawLotsExisting(node, &movable, &winrate);
    struct Node* raffle_new = DrawLotsNew(node, &movable, &winrate);

    if (raffle_new == NULL)
    {
        return raffle_existing;
    }
    else
    {
        return raffle_new;
    }
}

char Update(struct Node* node, char result, double value)
{
    char next;

    switch (result)
    {
        case 'w':
            node->a += value;
            next = 'l';
            break;
        
        case 'l':
            node->b += value;
            next = 'w';
            break;

        case 'd':
            node->a += value / 2;
            node->b += value / 2;
            next = 'd';
            break;

        default:
            next = '?';
    }

    assert(next != '?');
    return next;
}

char End(struct Node* node, double value)
{
    char result;

    int count_m = (int)_popcnt64(node->m);
    int count_y = (int)_popcnt64(node->y);

    if (count_m > count_y)
    {
        result = Update(node, 'w', value);
    }
    else if (count_m < count_y)
    {
        result = Update(node, 'l', value);
    }
    else
    {
        result = Update(node, 'd', value);
    }

    return result;
}

char PlayOut(struct Node* node, double *value, double learning_rate)
{
    char result;
    unsigned long long movable = GetMovable(node->m, node->y);

    if (movable)
    {
        struct Node* child = Move(node, movable);
        result = PlayOut(child, value, learning_rate);
        result = Update(node, result, *value);
    }
    else if (GetMovable(node->y, node->m))
    {
        if (node->child == NULL)
        {
            node->child = CreateNode(node->y, node->m);
        }
        result = PlayOut(node->child, value, learning_rate);
        result = Update(node, result, *value);
    }
    else
    {
        *value = 1.0;
        result = End(node, *value);
    }

    *value *= learning_rate;
    return result;
}

/*void Test1PlayOut()
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
}*/