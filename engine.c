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

    int a;
    int b;

    int pass;
    int stone;

    struct Sample *head;
    struct Sample *tail;

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

    node->pass = 0;
    node->stone = (int)pow((double)_popcnt64(m | y), LEARNING_RATE);

    node->head = NULL;
    node->tail = NULL;

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
        AddChild(node, raffle);
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

char Update(struct Node* node, char result)
{
    struct Sample *sample = CreateSample(result);
    if (node->head == NULL)
    {
        node->head = sample;
    }
    else if (node->tail == NULL)
    {
        node->head->next = sample;
        node->tail = sample;
    }
    else
    {
        node->tail->next = sample;
        node->tail = sample;
    }

    node->pass += 1;
    if (node->pass > node->stone)
    {
        struct Sample *pop = node->head;
        switch (pop->result)
        {
            case 'w':
                node->a -= 1;
                break;
            case 'l':
                node->b -= 1;
                break;
            case 'd':
                break;
            default:
                assert(0);
        }
        node->head = node->head->next;
        free(pop);
    }

    char next;
    switch (result)
    {
        case 'w':
            node->a += 1;
            next = 'l';
            break;
        
        case 'l':
            node->b += 1;
            next = 'w';
            break;

        case 'd':
            next = 'd';
            break;

        default:
            assert(0);
    }
    return next;
}

char End(struct Node* node)
{
    char result;

    int count_m = (int)_popcnt64(node->m);
    int count_y = (int)_popcnt64(node->y);

    if (count_m > count_y)
    {
        result = Update(node, 'w');
    }
    else if (count_m < count_y)
    {
        result = Update(node, 'l');
    }
    else
    {
        result = Update(node, 'd');
    }

    return result;
}

char PlayOut(struct Node* node)
{
    char result;
    unsigned long long movable = GetMovable(node->m, node->y);

    if (movable)
    {
        struct Node* child = Move(node, movable);
        result = PlayOut(child);
        result = Update(node, result);
    }
    else if (GetMovable(node->y, node->m))
    {
        if (node->child == NULL)
        {
            node->child = CreateNode(node->y, node->m);
        }
        result = PlayOut(node->child);
        result = Update(node, result);
    }
    else
    {
        result = End(node);
    }

    return result;
}