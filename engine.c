#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <x86intrin.h>
#include <math.h>
#include <assert.h>
#include "rule.c"
#include "sampling.c"

double LEARNING_RATE;

struct Node
{
    unsigned long long m;
    unsigned long long y;

    double a;
    double b;

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
        AddChild(node, raffle); // ここのループを省きたい
    }

    return raffle;
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

double Update(struct Node* node, char *result, double value)
{
    switch (*result)
    {
        case 'w':
            node->a += value;
            *result = 'l';
            break;
        
        case 'l':
            node->b += value;
            *result = 'w';
            break;

        case 'd':
            node->a += value / 2;
            node->b += value / 2;
            *result = 'd';
            break;

        default:
            assert(0);
    }
    return value * LEARNING_RATE;
}

double End(struct Node* node, char *result)
{
    int count_m = (int)_popcnt64(node->m);
    int count_y = (int)_popcnt64(node->y);

    if (count_m > count_y)
    {
        *result = 'w';
    }
    else if (count_m < count_y)
    {
        *result = 'l';
    }
    else
    {
        *result = 'd';
    }

    return Update(node, result, 1.0);
}

double PlayOut(struct Node* node, char *result)
{
    double value;
    unsigned long long movable = GetMovable(node->m, node->y);

    if (movable)
    {
        struct Node* child = Move(node, movable);
        value = PlayOut(child, result);
        value = Update(node, result, value);
    }
    else if (GetMovable(node->y, node->m))
    {
        if (node->child == NULL)
        {
            node->child = CreateNode(node->y, node->m);
        }
        value = PlayOut(node->child, result);
        value = Update(node, result, value);
    }
    else
    {
        value = End(node, result);
    }

    return value;
}