/*
 * MIT License
 *
 * Copyright (c) 2026, Felipe Martins Diel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "_oneflake.h"
#include "_constants.h"
#include "_exceptions.h"
#include "_generator.h"
#include "_decoder.h"

static int
oneflake_exec(PyObject *m)
{
    if (constants_init(m) < 0) {
        return -1;
    }
    if (exceptions_init(m) < 0) {
        return -1;
    }
    if (generator_init(m) < 0) {
        return -1;
    }
    if (decoder_init(m) < 0) {
        return -1;
    }

    return 0;
}

static PyModuleDef_Slot oneflake_module_slots[] = {
#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x030c0000
	{Py_mod_multiple_interpreters, Py_MOD_PER_INTERPRETER_GIL_SUPPORTED},
#endif
#if !defined(Py_LIMITED_API) && defined(Py_GIL_DISABLED)
	{Py_mod_gil, Py_MOD_GIL_NOT_USED},
#endif
    {Py_mod_exec, oneflake_exec},
    {0, NULL}
};

/* Module definition */
static PyModuleDef oneflake_module_def = {
    PyModuleDef_HEAD_INIT,
    .m_name = "oneflake._oneflake",
    .m_doc = "C implementation of Oneflake.",
    .m_size = 0,
    .m_methods = NULL,
    .m_slots = oneflake_module_slots
};

/* Initialize module */
PyMODINIT_FUNC
PyInit__oneflake(void)
{
    return PyModuleDef_Init(&oneflake_module_def);
}
