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

struct Node* GetChild(struct Node* node, unsigned long long movable, int index)
{
    struct Node* child = node->child;

    for (int i = 0; i < index; i++)
    {
        if (child != NULL)
        {
            movable ^= (movable & (child->m | child->y));
            child = child->next;
        }
        else
        {
            movable &= (movable - 1);
        }
    }
    if (child == NULL)
    {
        unsigned long long move = movable ^ (movable & (movable - 1));
        unsigned long long reversable = GetReversable(node->m, node->y, move);
        unsigned long long m = node->y ^ reversable;
        unsigned long long y = node->m | move | reversable;
        child = CreateNode(m, y);
    }

    return child;
}

void Test0GetChild()
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    unsigned long long movable = GetMovable(node->m, node->y);
    int index = 0;

    struct Node* child = GetChild(node, movable, index);
    if (node->child == NULL)
    {
        printf("Error: Failed AppEnd.\n");
    }
}

void Test1GetChild()
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    unsigned long long movable = GetMovable(node->m, node->y);
    int index = 0;

    struct Node* child = GetChild(node, movable, index);
    if (child->m != 68719476736)
    {
        printf("Error: child->m is wrong.\n");
    }
    if (child->y != 34762915840)
    {
        printf("Error: child->y is wrong.\n");
    }
}

void Test2GetChild()
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    long long movable = GetMovable(node->m, node->y);
    int index = 3;

    struct Node* child = GetChild(node, movable, index);
    if (child->m != 134217728)
    {
        printf("Error: child->m is wrong. ");
        printf("%llu\n", child->m);
    }
    if (child->y != 17695533694976)
    {
        printf("Error: child->y is wrong. ");
        printf("%llu\n", child->y);
    }
}

void Test3GetChild()
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    node->child = CreateNode(68719476736, 34762915840);
    long long movable = GetMovable(node->m, node->y);
    int index = 0;

    struct Node* child = GetChild(node, movable, index);
    if (child->m != 68719476736)
    {
        printf("Error: child->m is wrong. ");
        printf("%llu\n", child->m);
    }
    if (child->y != 34762915840)
    {
        printf("Error: child->y is wrong. ");
        printf("%llu\n", child->y);
    }
}

void Test4GetChild()
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    node->child = CreateNode(68719476736, 34762915840);
    long long movable = GetMovable(node->m, node->y);
    int index = 3;

    struct Node* child = GetChild(node, movable, index);
    if (child->m != 134217728)
    {
        printf("Error: child->m is wrong. ");
        printf("%llu\n", child->m);
    }
    if (child->y != 17695533694976)
    {
        printf("Error: child->y is wrong. ");
        printf("%llu\n", child->y);
    }
}

PyObject *TestGetChild(PyObject *self, PyObject *args)
{
    Test0GetChild();
    Test1GetChild();
    Test2GetChild();
    Test3GetChild();
    Test4GetChild();

    // append child?

    printf("TestGetChild done.\n");
    return Py_None;
}

int ChoiceChild(struct Node* node, unsigned long long movable)
{
    int index = 0;
    int count = 0;
    double winrate = 1.0;
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
            node->result = 'w';
            return index;
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

        if (sample < winrate)
        {
            index = count;
            winrate = sample;
        }
        count++;
        movable ^= (movable & (child->m | child->y));
        child = child->next;
    }

    while (movable)
    {
        win = 0;
        sample = SampleUniform();

        if (sample < winrate)
        {
            index = count;
            winrate = sample;
        }
        count++;
        movable &= (movable - 1);
    }

    if (win)
    {
        if (draw) node->result = 'd';
        else node->result = 'l';
    }

    return index;
}

PyObject *TestChoiceChild(PyObject *self, PyObject *args)
{
    struct Node* node = CreateNode(34628173824, 68853694464);
    unsigned long long movable = GetMovable(node->m, node->y);

    int seed = rand() % 100;
    for (int i = 0; i < seed; i++)
    {
        SampleUniform();
    }
    node->child = CreateNode(68719476736, 34762915840);
    node->child->b = 9.0;
    for (int i = 0; i < 20; i++)
    {
        printf("%d\n", ChoiceChild(node, movable));
    }

    printf("\n");
    node->child->result = 'l';
    for (int i = 0; i < 20; i++)
    {
        printf("%d\n", ChoiceChild(node, movable));
    }
    if (node->result != 'w')
    {
        printf("Error\n");
    }

    // if children are all win.

    printf("TestChoiceChild done.\n");
    return Py_None;
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
        if (node->result = 'w') result = Update(node, 1.0);
        else result = Update(node, -1.0);
        return result;
    }
    else
    {
        unsigned long long movable = GetMovable(node->m, node->y);
        if (movable)
        {
            int index = ChoiceChild(node, movable);
            struct Node* child = GetChild(node, movable, index);
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

PyObject *TestPlayOut(PyObject *self, PyObject *args)
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
    {"TestGetChild", TestGetChild, METH_VARARGS},
    {"TestChoiceChild", TestChoiceChild, METH_VARARGS},
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