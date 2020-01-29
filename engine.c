#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <intrin.h>
#include <math.h>
#include <stdint.h>

double learning_rate;

unsigned long long GetMovableL
(unsigned long long player, unsigned long long masked, unsigned long long blank, int dir)
{
    unsigned long long tmp;
    tmp = masked & (player << dir);
    tmp |= masked & (tmp << dir);
    tmp |= masked & (tmp << dir);
    tmp |= masked & (tmp << dir);
    tmp |= masked & (tmp << dir);
    tmp |= masked & (tmp << dir);

    return blank & (tmp << dir);
}

unsigned long long GetMovableR
(unsigned long long player, unsigned long long masked, unsigned long long blank, int dir)
{
    unsigned long long tmp;
    tmp = masked & (player >> dir);
    tmp |= masked & (tmp >> dir);
    tmp |= masked & (tmp >> dir);
    tmp |= masked & (tmp >> dir);
    tmp |= masked & (tmp >> dir);
    tmp |= masked & (tmp >> dir);

    return blank & (tmp >> dir);
}

unsigned long long GetMovable(unsigned long long m, unsigned long long y)
{
    unsigned long long blank = ~(m | y);
    unsigned long long h = y & 0x7e7e7e7e7e7e7e7e;
    unsigned long long v = y & 0x00ffffffffffff00;
    unsigned long long a = y & 0x007e7e7e7e7e7e00;
    unsigned long long legal;
    legal = GetMovableL(m, h, blank, 1);
    legal |= GetMovableL(m, v, blank, 8);
    legal |= GetMovableL(m, a, blank, 7);
    legal |= GetMovableL(m, a, blank, 9);
    legal |= GetMovableR(m, h, blank, 1);
    legal |= GetMovableR(m, v, blank, 8);
    legal |= GetMovableR(m, a, blank, 7);
    legal |= GetMovableR(m, a, blank, 9);

    return legal;
}

unsigned long long GetReversableL
(unsigned long long player, unsigned long long blank_masked, unsigned long long site, int dir)
{
    unsigned long long rev = 0;
    unsigned long long tmp = ~(player | blank_masked) & (site << dir);

    if (tmp)
    {
        for (int i = 0; i < 6; i++)
        {
            tmp <<= dir;
            if (tmp & blank_masked)
            {
                break;
            }
            else if (tmp & player)
            {
                rev |= tmp >> dir;
                break;
            }
            else
            {
                tmp |= tmp >> dir;
            }
        }
    }

    return rev;
}

unsigned long long GetReversableR
(unsigned long long player, unsigned long long blank_masked, unsigned long long site, int dir)
{
    unsigned long long rev = 0;
    unsigned long long tmp = ~(player | blank_masked) & (site >> dir);

    if (tmp)
    {
        for (int i = 0; i < 6; i++)
        {
            tmp >>= dir;
            if (tmp & blank_masked)
            {
                break;
            }
            else if (tmp & player)
            {
                rev |= tmp << dir;
                break;
            }
            else
            {
                tmp |= tmp << dir;
            }
        }
    }

    return rev;
}

unsigned long long GetReversable
(unsigned long long m, unsigned long long y, unsigned long long move)
{
    unsigned long long blank_h = ~(m | y & 0x7e7e7e7e7e7e7e7e);
    unsigned long long blank_v = ~(m | y & 0x00ffffffffffff00);
    unsigned long long blank_a = ~(m | y & 0x007e7e7e7e7e7e00);
    unsigned long long rev;
    rev = GetReversableL(m, blank_h, move, 1);
    rev |= GetReversableL(m, blank_v, move, 8);
    rev |= GetReversableL(m, blank_a, move, 7);
    rev |= GetReversableL(m, blank_a, move, 9);
    rev |= GetReversableR(m, blank_h, move, 1);
    rev |= GetReversableR(m, blank_v, move, 8);
    rev |= GetReversableR(m, blank_a, move, 7);
    rev |= GetReversableR(m, blank_a, move, 9);
    
    return rev;
}

double SampleUniform()
{
    static uint64_t x = 88172645463325252ULL;
    x = x ^ (x << 7);
    x = x ^ (x >> 9);
    return (double)x / (double)UINT64_MAX;
}

PyObject *SetSeed(PyObject *self, PyObject *args)
{
    int seed;
    if (!PyArg_ParseTuple(args, "i", &seed))
    {
        return NULL;
    }
    for (int i = 0; i < seed; i++)
    {
        SampleUniform();
    }
    return Py_None;
}

double SampleNormal()
{
    double sum = 0;
    for (int i = 0; i < 12; i++)
    {
        sum += SampleUniform();
    }
    return sum - 6.0;
}

double SampleGamma(double alpha)
{
    double c1 = alpha - 1.0 / 3.0;
    double c2 = 1.0 / sqrt(9.0 * c1);
    double norm;
    double v;
    double u;
    while (1)
    {
        norm = SampleNormal();
        if (c2 * norm <= -1.0) continue;
        v = pow(1.0 + c2 * norm, 3.0);
        u = SampleUniform();
        if (u < 1.0 - 0.331 * pow(norm, 4.0)) return c1 * v;
        if (log(u) < 0.5 * pow(norm, 2.0) + c1 * (1.0 - v + log(v))) return c1 * v;
    }
}

double SampleBeta(double a, double b)
{
    double gamma1 = SampleGamma(a);
    double gamma2 = SampleGamma(b);
    return gamma1 / (gamma1 + gamma2);
}

PyObject *WrapSampleBeta(PyObject *self, PyObject *args)
{
    double a;
    double b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
    {
        return NULL;
    }
    return Py_BuildValue("d", SampleBeta(a, b));
}

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
    node->result = 'n';
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
    int win = 1;
    int draw = 0;

    while (child != NULL)
    {
        if (child->result == 'w')
        {
            sample = 1.0;
        }
        else if (child->result == 'l')
        {
            win = 0;
            sample = 0.0;
            node->result = 'w';
        }
        else if (child->result == 'd')
        {
            sample = 0.5;
            draw = 1;
        }
        else
        {
            win = 0;
            sample = SampleBeta(child->a, child->b);
        }

        if (sample <= *winrate)
        {
            choice = child;
            *winrate = sample;
        }

        *movable ^= (*movable & (child->m | child->y));
        child = child->next;
    }

    if (*movable == 0)
    {
        if (win)
        {
            if (draw) node->result = 'd';
            else node->result = 'l';
        }
    }

    return choice;
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

PyObject *TestDrawLotsExisting(PyObject *self, PyObject *args)
{
    printf("TestDrawLotsExisting start.\n");

    Test1DrawLotsExisting();
    Test2DrawLotsExisting();
    Test3DrawLotsExisting();    

    printf("TestDrawLotsExisting done.\n");
    return Py_None;
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

PyObject *TestDrawLotsNew(PyObject *self, PyObject *args)
{
    printf("TestDrawLotsNew start.\n");

    Test1DrawLotsNew();
    Test2DrawLotsNew();

    printf("TestDrawLotsNew done.\n");
    return Py_None;
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

double Update(struct Node* node, double result)
{
    double value = fabs(result);
    if (result > 0)
    {
        node->a += value;
        return -(value * learning_rate);
    }
    else
    {
        node->b += value;
        return value * learning_rate;
    }
}

PyObject *TestUpdate(PyObject *self, PyObject *args)
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    learning_rate = 0.9;

    Update(node, 1.0);
    if (node->a != 2.0)
    {
        printf("Error\n");
    }
    Update(node, -1.0);
    if (node->b != 2.0)
    {
        printf("Error\n");
    }
    if (Update(node, 1.0) != -0.9)
    {
        printf("Error\n");
    }
    if (Update(node, -1.0) != 0.9)
    {
        printf("Error\n");
    }

    printf("TestUpdate done.\n");
    return Py_None;
}

double End(struct Node* node)
{
    double result;
    int count_m = (int)__popcnt64(node->m);
    int count_y = (int)__popcnt64(node->y);
    if (count_m > count_y)
    {
        node->result = 'w';
        result = Update(node, 1.0);
    }
    else if (count_m < count_y)
    {
        node->result = 'l';
        result = Update(node, -1.0);
    }
    else
    {
        node->result = 'd';
        result = Update(node, -1.0);
    }
    return result;
}

double PlayOut(struct Node* node, int depth)
{
    double result;
    if (node->result != 'n')
    {
        if (node->result == 'w') result = Update(node, 1.0);
        else result = Update(node, -1.0);
        return result;
    }
    else
    {
        unsigned long long movable = GetMovable(node->m, node->y);
        if (movable)
        {
            struct Node* child = Move(node, movable);
            result = PlayOut(child, depth + 1);
            result = Update(node, result);
        }
        else if (GetMovable(node->y, node->m))
        {
            if (node->child == NULL)
            {
                node->child = CreateNode(node->y, node->m);
            }
            result = PlayOut(node->child, depth + 1);
            result = Update(node, result);
        }
        else
        {
            result = End(node);
        }
        return result;
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

PyObject *TestPlayOut(PyObject *self, PyObject *args)
{
    printf("TestPlayOut start.\n");

    Test1PlayOut();
    Test2PlayOut();

    printf("TestPlayOut done.\n");
    return Py_None;
}

// can be static?
// All processes are same seeds?
PyObject *Search(PyObject *self, PyObject *args)
{
    unsigned long long m;
    unsigned long long y;
    int trial;

    if (!PyArg_ParseTuple(args, "KKdi", &m, &y, &learning_rate, &trial))
    {
        return NULL;
    }

    struct Node* node = CreateNode(m, y);

    for (int j = 0; j < trial; j++)
    {  
        //printf("\r%d/%d", j, trial);
        PlayOut(node, 1);
    }
    //printf("\r\n");

    double winrate = node->a / (node->a + node->b);
    Free(node);

    return Py_BuildValue("d", winrate);
}

static PyMethodDef engine_methods[] = {
    {"SetSeed", SetSeed, METH_VARARGS},
    {"TestDrawLotsExisting", TestDrawLotsExisting, METH_VARARGS},
    {"TestDrawLotsNew", TestDrawLotsNew, METH_VARARGS},
    {"TestUpdate", TestUpdate, METH_VARARGS},
    {"TestPlayOut", TestPlayOut, METH_VARARGS},
    {"Search", Search, METH_VARARGS},
    {"WrapSampleBeta", WrapSampleBeta, METH_VARARGS},
    {NULL}
};

static struct PyModuleDef engine_module = {
    PyModuleDef_HEAD_INIT,
    "engine",
    "Engine of Othello",
    -1,
    engine_methods
};

PyMODINIT_FUNC PyInit_engine(void)
{
    return PyModule_Create(&engine_module);
}