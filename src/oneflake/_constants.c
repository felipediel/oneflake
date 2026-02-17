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
#include "_constants.h"

int
constants_init(PyObject *module)
{
    PyObject *unix_epoch_ms_obj = NULL;
    PyObject *min_epoch_ms_obj = NULL;
    PyObject *max_epoch_ms_obj = NULL;
    PyObject *default_worker_id_obj = NULL;
    PyObject *default_time_unit_ns_obj = NULL;
    PyObject *default_epoch_ms_obj = NULL;

    if (PyModule_AddIntConstant(module, "LAYOUT_TWS", LAYOUT_TWS) < 0) {
        return -1;
    }

    if (PyModule_AddIntConstant(module, "LAYOUT_TSW", LAYOUT_TSW) < 0) {
        return -1;
    }

    unix_epoch_ms_obj = PyLong_FromUnsignedLongLong(UNIX_EPOCH_MS);
    if (unix_epoch_ms_obj == NULL) {
        return -1;
    }
    if (PyModule_AddObject(module, "UNIX_EPOCH_MS", unix_epoch_ms_obj) < 0) {
        Py_DECREF(unix_epoch_ms_obj);
        return -1;
    }

    min_epoch_ms_obj = PyLong_FromUnsignedLongLong(MIN_EPOCH_MS);
    if (min_epoch_ms_obj == NULL) {
        return -1;
    }
    if (PyModule_AddObject(module, "MIN_EPOCH_MS", min_epoch_ms_obj) < 0) {
        Py_DECREF(min_epoch_ms_obj);
        return -1;
    }

    max_epoch_ms_obj = PyLong_FromUnsignedLongLong(MAX_EPOCH_MS);
    if (max_epoch_ms_obj == NULL) {
        return -1;
    }
    if (PyModule_AddObject(module, "MAX_EPOCH_MS", max_epoch_ms_obj) < 0) {
        Py_DECREF(max_epoch_ms_obj);
        return -1;
    }

    default_worker_id_obj = PyLong_FromUnsignedLongLong(DEFAULT_WORKER_ID);
    if (default_worker_id_obj == NULL) {
        return -1;
    }
    if (PyModule_AddObject(
        module, "DEFAULT_WORKER_ID", default_worker_id_obj
    ) < 0) {
        Py_DECREF(default_worker_id_obj);
        return -1;
    }

    if (PyModule_AddIntConstant(
        module, "DEFAULT_LAYOUT", DEFAULT_LAYOUT
    ) < 0) {
        return -1;
    }

    if (PyModule_AddIntConstant(
        module, "DEFAULT_TIMESTAMP_BITS", DEFAULT_TIMESTAMP_BITS
    ) < 0) {
        return -1;
    }

    if (PyModule_AddIntConstant(
        module, "DEFAULT_WORKER_BITS", DEFAULT_WORKER_BITS
    ) < 0) {
        return -1;
    }

    if (PyModule_AddIntConstant(
        module, "DEFAULT_SEQUENCE_BITS", DEFAULT_SEQUENCE_BITS
    ) < 0) {
        return -1;
    }

    default_time_unit_ns_obj = PyLong_FromUnsignedLongLong(
        DEFAULT_TIME_UNIT_NS
    );
    if (default_time_unit_ns_obj == NULL) {
        return -1;
    }
    if (PyModule_AddObject(
        module, "DEFAULT_TIME_UNIT_NS", default_time_unit_ns_obj
    ) < 0) {
        Py_DECREF(default_time_unit_ns_obj);
        return -1;
    }

    default_epoch_ms_obj = PyLong_FromUnsignedLongLong(DEFAULT_EPOCH_MS);
    if (default_epoch_ms_obj == NULL) {
        return -1;
    }
    if (PyModule_AddObject(
        module, "DEFAULT_EPOCH_MS", default_epoch_ms_obj
    ) < 0) {
        Py_DECREF(default_epoch_ms_obj);
        return -1;
    }

    return 0;
}
