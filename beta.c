#include <stdio.h>
#include <stdlib.h>

int UNIFORM_RANGE = 5;

struct Uniform
{
    float value;
    struct Uniform *prev;
    struct Uniform *next;
};

struct Position
{
    int a;
    int b;
    struct Uniform *sample;
};

struct Uniform* CreateUniform(float uniform, struct Uniform *prev, struct Uniform *next)
{
    struct Uniform *node = (struct Uniform *)malloc(sizeof(struct Uniform));
    node->value = uniform;
    node->prev = prev;
    node->next = next;

    return node;
}

// Uniformを追加する処理
// RANGE外の処理は別
void AddUniform(struct Uniform *node, float uniform, char direction)
{
    struct Uniform *target = node;

    switch (direction)
    {
        case 'l':
            // 終了条件
            if (target == NULL)
            {
                target = CreateUniform(uniform, NULL, target->next);
            }
            else if (target->value < uniform)
            {
                struct Uniform *new = CreateUniform(uniform, target, target->next);
                target->next = new;
                target->next->prev = new;
            }
            else
            {
                AddUniform(target->prev, uniform, 'l');
            }
            break;
        
        case 'r':
            // 終了条件
            if (target == NULL)
            {
                target = CreateUniform(uniform, target->prev, NULL);
            }
            else if (uniform < target->value)
            {
                struct Uniform *new = CreateUniform(uniform, target->prev, target);
                target->prev->next = new;
                target->prev = new;
            }
            else
            {
                AddUniform(target->next, uniform, 'r');
            }
            break;
    }
}

void MoveUniform(struct Uniform *current, char result, char direction)
{
    switch (result)
    {
        case 'w':
            if (direction == 'r')
            {
                current = current->next;
            }
            break;

        case 'l':
            if (direction == 'l')
            {
                current = current->next;
            }
            break;
    }
}

void ClearRangeUniform(struct Uniform *node, int range, char direction)
{
    // 終了条件
    if (node == NULL) return;
    
    switch (direction)
    {
        case 'l':
            ClearRangeUniform(node->prev, range++, direction);
            break;
        case 'r':
            ClearRangeUniform(node->next, range++, direction);
            break;
    }

    if (range > UNIFORM_RANGE)
    {
        free(node);
    }
}

// プレイアウト後に実行
// 構造体が持つサンプルメンバを更新する
struct Uniform* UpdateSample(struct Uniform *node, int result)
{
    float uniform = (float)rand() / RAND_MAX;

    if (node == NULL)
    {
        return CreateUniform(uniform, NULL, NULL);
    }

    char direction;
    if (uniform > node->value)
    {
        direction = 'r';
    }
    else
    {
        direction = 'l';
    }

    AddUniform(node, uniform, direction); //ここどうなんだろ？nodeじゃないものを操作しているような…
    MoveUniform(node, result, direction);
    ClearRangeUniform(node, 0, direction);
}

void AccessUniform(struct Uniform *node)
{
    struct Uniform *current = node;
    while (current->prev != NULL)
    {
        current = current->prev;
    }
    while (current != NULL)
    {
        printf("%f\n", current->value);
        current = current->next;
    }
}

struct Position* CreatePosition(int a, int b)
{
    struct Position *node = (struct Position *)malloc(sizeof(struct Position));
    node->a = a;
    node->b = b;
    node->sample = NULL;
    for (int i = 0; i < a; i++)
    {
        node->sample = UpdateSample(node->sample, 'w');
    }
    for (int i = 0; i < b; i++)
    {
        node->sample = UpdateSample(node->sample, 'l');
    }

    return node;
}

char Playout(float prob)
{
    float uniform = (float)rand() / RAND_MAX;
    if (uniform < prob)
    {
        return 'w';
    }
    else
    {
        return 'l';
    }
}

int main(int argc, char *argv[])
{
    printf("こんにちは\n");
    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    int seed = atoi(argv[3]);
    printf("Seedは%dです\n", seed);
    srand(seed);

    struct Position *hoge = CreatePosition(a, b);
    printf("リストを構成しました。\n");

    //AccessUniform(hoge->sample);

    int trial = 1000;
    float prob = 0.5;
    for (int i = 0; i < trial; i++)
    {
        int result = Playout(prob);
        hoge->sample = UpdateSample(hoge->sample, result);
        printf("%f\n", hoge->sample->value);
    }

    /*
    trial = 400;
    prob = 0.3;
    for (int i = 0; i < trial; i++)
    {
        int result = Playout(prob);
        hoge->sample = UpdateSample(hoge->sample, result);
        printf("%f\n", hoge->sample->value);
    }
    */

    return 0;
}