#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <intrin.h>
#include <math.h>
#include <time.h>
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
    if ((a == 1.0) && (b == 1.0))
    {
        return SampleUniform();
    }
    else
    {
        double gamma1 = SampleGamma(a);
        double gamma2 = SampleGamma(b);
        return gamma1 / (gamma1 + gamma2);
    }
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

void AddChild(struct Node *node, struct Node *child)
{
    if (node->child == NULL)
    {
        node->child = child;
    }
    else
    {
        struct Node *last = node->child;
        while (last->next != NULL)
        {
            last = last->next;
        }
        last->next = child;
    }
}

void FindChildrenAndEnd(struct Node *node)
{
    if (node->result != 'n') return;
    if (node->child != NULL) return;
    
    // signed for bit computation
    long long movable = GetMovable(node->m, node->y);
    if (movable)
    {
        unsigned long long move;
        unsigned long long reversable;
        unsigned long long m;
        unsigned long long y;
        //struct Node* last = node->child; Why cannot work?

        while (movable)
        {
            move = (unsigned long long)(movable & -movable);
            reversable = GetReversable(node->m, node->y, move);
            m = node->m | move | reversable;
            y = node->y ^ reversable;
            AddChild(node, CreateNode(y, m));
            movable ^= move;
        }
    }
    else if (GetMovable(node->y, node->m))
    {
        node->child = CreateNode(node->y, node->m);
    }
    else
    {
        int count_m = (int)__popcnt64(node->m);
        int count_y = (int)__popcnt64(node->y);
        if (count_m > count_y) node->result = 'w';
        else if (count_m < count_y) node->result = 'l';
        else node->result = 'd';
    }
}

int PickOrDeleteChild(struct Node* node)
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
            //printf("%s\n", "Cut Node.");
            node->result = 'w';
            return count;
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
            //sample = SampleBetaSimple(child->a, child->b);
            // Warning for simple sample,
            // can be more than 1.0 and less than 0.0
        }

        if (sample < winrate)
        {
            index = count;
            winrate = sample;
        }
        child = child->next;
        count++;
    }

    if (win)
    {
        //printf("%s\n", "Cut Node.");
        if (draw) node->result = 'd';
        else node->result = 'l';
    }

    return index;
}

double Update(struct Node* node, double result)
{
    double value = fabs(result) * learning_rate;
    if (result > 0)
    {
        node->a += value;
        return -value;
    }
    else
    {
        node->b += value;
        return value;
    }
}

double PlayOut(struct Node* node, int depth)
{
    FindChildrenAndEnd(node);

    if (node->result != 'n')
    {
        double value = 1.0;
        if (node->result == 'w') 
        {
            node->a += value;
            return -value;
        }
        else if (node->result == 'l')
        {
            node->b += value;
            return value;
        }
        else
        {
            return 0.0;
        }
    }
    else
    {
        int index = PickOrDeleteChild(node);
        struct Node* child = node->child;
        for (int i = 0; i < index; i++)
        {
            child = child->next;
        }
        double result = PlayOut(child, depth + 1);
        result = Update(node, result);
        return result;
    }
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