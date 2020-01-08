#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>

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

double SampleNormal()
{
    int sum = 0;
    for (int i = 0; i < 12; i++)
    {
        sum += rand();
    }
    return (double)sum / (double)RAND_MAX - 6.0;
}

double SampleGamma(double alpha)
{
    double c1 = alpha - 1.0 / 3.0;
    double c2 = 1.0 / sqrt(9.0 * c1);
    while (1)
    {
        double norm = SampleNormal();
        if (c2 * norm <= -1.0) continue;
        double v = pow(1.0 + c2 * norm, 3.0);
        double u = (double)rand() / (double)RAND_MAX;
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

struct Node
{
    unsigned long long m;
    unsigned long long y;
    double a;
    double b;
    struct Node *child;
    struct Node *next;
};

int PopCount(unsigned long long x)
{
    int count = 0;

    for (int i = 0; i < 64; i++)
    {
        if (x % 2)
        {
            count += 1;
        }
        x >>= 1;
    }

    return count;
}

struct Node* CreateNode(unsigned long long m, unsigned long long y)
{
    struct Node* node;
    node = (struct Node*)malloc(sizeof(struct Node));

    node->m = m;
    node->y = y;
    double count = pow(65.0 - (double)PopCount(m | y), 2.0);
    node->a = count;
    node->b = count;
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

void FindChildren(struct Node *node)
{
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
}

int ChoiceChild(struct Node* node)
{
    int index;
    int count = 0;
    double winrate = 1.0;
    struct Node* child = node->child;
    double sample;

    while (child != NULL)
    {
        sample = SampleBeta(child->a, child->b);
        if (sample < winrate)
        {
            index = count;
            winrate = sample;
        }
        child = child->next;
        count++;
    }

    return index;
}

char PlayOut(struct Node* node)
{
    char state;
    FindChildren(node);

    if (node->child != NULL)
    {
        int index = ChoiceChild(node);
        struct Node* child = node->child;
        for (int i = 0; i < index; i++)
        {
            child = child->next;
        }
        state = PlayOut(child);
        if (state == 'w') state = 'l';
        else if (state == 'l') state = 'w';
    }
    else
    {
        int count_m = PopCount(node->m);
        int count_y = PopCount(node->y);
        if (count_m > count_y) state = 'w';
        else if (count_m < count_y) state = 'l';
    }

    if (state == 'w') node->a += 1.0;
    else if (state == 'l') node->b += 1.0;
    else
    {
        node->a += 0.5;
        node->b += 0.5;
    }

    return state;
}

PyObject *Search(PyObject *self, PyObject *args)
{
    unsigned long long m;
    unsigned long long y;
    int trial;

    if (!PyArg_ParseTuple(args, "KKi", &m, &y, &trial))
    {
        return NULL;
    }

    struct Node* node = CreateNode(m, y);
    for (int i = 0; i < trial; i++)
    {
        PlayOut(node);
    }

    // no movable case
    int index = ChoiceChild(node);
    struct Node* child = node->child;
    for (int i = 0; i < index; i++)
    {
        child = child->next;
    }
    m = child->y;
    y = child->m;
    printf("%lf\n", child->b / (child->a + child->b));
    Free(node);

    return Py_BuildValue("KK", m, y);
}

static PyMethodDef engine_methods[] = {
    {"Search", Search, METH_VARARGS},
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