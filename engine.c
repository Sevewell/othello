#define PY_SSIZE_T_CLEAN
#include <Python.h>

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

PyObject *GetMovable(PyObject *self, PyObject *args)
{
    unsigned long long m;
    unsigned long long y;

    if (!PyArg_ParseTuple(args, "KK", &m, &y))
    {
        return NULL;
    }

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

    return Py_BuildValue("K", legal);
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

PyObject *GetReversable(PyObject *self, PyObject *args)
{
    unsigned long long m;
    unsigned long long y;
    unsigned long long move;

    if (!PyArg_ParseTuple(args, "KKK", &m, &y, &move))
    {
        return NULL;
    }

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
    
    return Py_BuildValue("K", rev);
}

static PyMethodDef engine_methods[] = {
    {"GetMovable", GetMovable, METH_VARARGS},
    {"GetReversable", GetReversable, METH_VARARGS},
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