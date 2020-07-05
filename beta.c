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
void AddUniform(struct Uniform *target, float uniform, char direction)
{
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

struct Uniform* MoveUniform(struct Uniform current, char result, char direction)
{
    switch (result)
    {
        case 'w':
            if (direction == 'l')
            {
                node = node->prev;
            }
            break;

        case 'l':
            if (direction == 'r')
            {
                node = node->next;
            }
            break;
    }

    return node;
}

// プレイアウト後に実行
// 構造体が持つサンプルメンバを更新する
struct Uniform* UpdateSample(struct Uniform *node, int result)
{
    struct Uniform *new = CreateUniform(NULL, NULL);

    if (node == NULL)
    {
        return new;
    }

    struct Uniform *target = node;
    int range = UNIFORM_RANGE;

    // uniformを追加する処理と勝敗によってノードを移動する処理を分けたい
    // RANGE外のノードを削除する処理も分けたい
    // まず向きを定める
    if (new->value > target->value)
    {
        switch (result)
        {
            case 'w':
                // RANGEより先を奥から消していきたい
                // 再帰関数が良さそう
                if (range < 0)
                {

                }
                else if (target == NULL)
                {
                    target = new;
                    node = target;
                }
                else if (target->value > new->value)
                {
                    target->prev->next = new;
                    new->prev = target->prev;
                    new->next = target;
                    target->prev = new;
                    node = new;
                }
                else
                {
                    target = target->next;
                    range--;
                }
                break;

            case 'l':
        
        }
        /*
        while (1)
        {
            if (target->value > new->value)
            {
                target->next->prev = new;
                new->next = target->next;
                new->prev = target;
                target->next = new;
                break;
            }
            if (target->prev == NULL)
            {
                target->prev = new;
                new->next = target;
                break;
            }
            target = target->prev;
            // ループ数を制限したい
        }
        return MoveUniform(node, result, 'l');
        */
    }
    else
    {
        while (1)
        {
            if (target->value < new->value)
            {
                target->prev->next = new;
                new->prev = target->prev;
                new->next = target;
                target->prev = new;
                break;
            }
            if (target->next == NULL)
            {
                target->next = new;
                new->prev = target;
                break;
            }
            target = target->next;
        }
        return MoveUniform(node, result, 'r');
    }
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