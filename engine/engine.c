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
    uint64_t m;
    uint64_t y;

    float a;
    float b;

    struct Node *child;
    struct Node *next;
};

struct Node* CreateNode(uint64_t m, uint64_t y)
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

struct Node* Move(struct Node* node, uint64_t movable)
{
    struct Node* choice = NULL;
    double winrate = 1.0;
    double sample;
    struct Node* child = node->child;

    while (child != NULL)
    {
        sample = SampleBetaFast((double)child->a, (double)child->b, SampleUINT64() % NUM_SAMPLES);
        if (sample <= winrate)
        {
            choice = child;
            winrate = sample;
        }
        movable ^= (movable & (child->m | child->y));
        child = child->next;
    }

    uint64_t move = 0;

    while (movable)
    {
        sample = SampleUniform();
        if (sample <= winrate)
        {
            move = movable ^ (movable & (movable - 1));
            winrate = sample;
        }
        movable &= (movable - 1);
    }

    if (move)
    {
        uint64_t reversable = GetReversable(node->m, node->y, move);
        choice = CreateNode(node->y ^ reversable, node->m | move | reversable);
        choice->next = node->child;
        node->child = choice;
    }

    return choice;
}

double Update(struct Node* node, char *result, double value)
{
    switch (*result)
    {
        case 'w':
            node->a += (float)value;
            *result = 'l';
            break;
        
        case 'l':
            node->b += (float)value;
            *result = 'w';
            break;

        case 'd':
            node->a += (float)value / 2;
            node->b += (float)value / 2;
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
    uint64_t movable = GetMovable(node->m, node->y);

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