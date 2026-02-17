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
#include "_exceptions.h"

PyObject *OneflakeException = NULL;
PyObject *ConfigurationError = NULL;
PyObject *ClockError = NULL;
PyObject *DecodeError = NULL;
PyObject *InvalidLayoutError = NULL;
PyObject *InvalidBitWidthError = NULL;
PyObject *InvalidWorkerError = NULL;
PyObject *InvalidTimeUnitError = NULL;
PyObject *InvalidEpochError = NULL;
PyObject *InvalidClockFunctionError = NULL;
PyObject *TimeBeforeEpochError = NULL;
PyObject *TimeReadError = NULL;
PyObject *ClockRollbackError = NULL;
PyObject *TimeOverflowError = NULL;
PyObject *LayoutDecodeError = NULL;

static int
create_exception(
    PyObject **exc_ptr,
    const char *qualified_name,
    PyObject *base
)
{
    if (*exc_ptr != NULL) {
        return 0;
    }

    *exc_ptr = PyErr_NewException(qualified_name, base, NULL);
    if (*exc_ptr == NULL) {
        return -1;
    }

    return 0;
}

static int
add_exception_to_module(
    PyObject *module,
    const char *name,
    PyObject *exc
)
{
    Py_INCREF(exc);
    if (PyModule_AddObject(module, name, exc) < 0) {
        Py_DECREF(exc);
        return -1;
    }

    return 0;
}

/* Set exception cause */
static void
set_exception_cause(
    PyObject *new_exc,
    PyObject *old_value,
    PyObject *old_tb
)
{
    if (old_tb != NULL)
        PyException_SetTraceback(old_value, old_tb);

    if (old_value != NULL) {
        Py_INCREF(old_value);
        PyException_SetCause(new_exc, old_value);
    }

    PyException_SetContext(new_exc, NULL);
}

/* Set exception from cause */
void
set_exception_from_cause(PyObject *exc_type, const char *msg)
{
    PyObject *old_type = NULL;
    PyObject *old_value = NULL;
    PyObject *old_tb = NULL;
    PyObject *new_exc = NULL;

    PyErr_Fetch(&old_type, &old_value, &old_tb);
    PyErr_NormalizeException(&old_type, &old_value, &old_tb);

    new_exc = PyObject_CallFunction(exc_type, "s", msg);
    if (new_exc == NULL) {
        // If creating the new exception fails, link that failure to the old one
        PyObject *fail_exc_type = NULL;
        PyObject *fail_exc_value = NULL;
        PyObject *fail_exc_tb = NULL;

        PyErr_Fetch(&fail_exc_type, &fail_exc_value, &fail_exc_tb);
        if (fail_exc_value && old_value) {
            set_exception_cause(fail_exc_value, old_value, old_tb);
        }

        PyErr_Restore(fail_exc_type, fail_exc_value, fail_exc_tb);

        Py_XDECREF(old_type);
        Py_XDECREF(old_tb);
        return;
    }

    if (old_value != NULL) {
        set_exception_cause(new_exc, old_value, old_tb);
    }

    PyErr_SetObject(exc_type, new_exc);

    Py_DECREF(new_exc);
    Py_XDECREF(old_type);
    Py_XDECREF(old_tb);
}

/* Initialize exceptions module */
int
exceptions_init(PyObject *module)
{
    if (create_exception(
        &OneflakeException,
        "oneflake.OneflakeException",
        PyExc_Exception
    ) < 0) {
        return -1;
    }

    if (create_exception(
        &ConfigurationError,
        "oneflake.ConfigurationError",
        OneflakeException
    ) < 0) {
        return -1;
    }
    if (create_exception(
        &ClockError,
        "oneflake.ClockError",
        OneflakeException
    ) < 0) {
        return -1;
    }
    if (create_exception(
        &DecodeError,
        "oneflake.DecodeError",
        OneflakeException
    ) < 0) {
        return -1;
    }
    if (create_exception(
        &InvalidLayoutError,
        "oneflake.InvalidLayoutError",
        ConfigurationError
    ) < 0) {
        return -1;
    }
    if (create_exception(
        &InvalidBitWidthError,
        "oneflake.InvalidBitWidthError",
        ConfigurationError
    ) < 0) {
        return -1;
    }
    if (create_exception(
        &InvalidWorkerError,
        "oneflake.InvalidWorkerError",
        ConfigurationError
    ) < 0) {
        return -1;
    }
    if (create_exception(
        &InvalidTimeUnitError,
        "oneflake.InvalidTimeUnitError",
        ConfigurationError
    ) < 0) {
        return -1;
    }
    if (create_exception(
        &InvalidEpochError,
        "oneflake.InvalidEpochError",
        ConfigurationError
    ) < 0) {
        return -1;
    }
    if (create_exception(
        &InvalidClockFunctionError,
        "oneflake.InvalidClockFunctionError",
        ConfigurationError
    ) < 0) {
        return -1;
    }
    if (create_exception(
        &TimeBeforeEpochError,
        "oneflake.TimeBeforeEpochError",
        ClockError
    ) < 0) {
        return -1;
    }
    if (create_exception(
        &TimeReadError,
        "oneflake.TimeReadError",
        ClockError
    ) < 0) {
        return -1;
    }
    if (create_exception(
        &ClockRollbackError,
        "oneflake.ClockRollbackError",
        ClockError
    ) < 0) {
        return -1;
    }
    if (create_exception(
        &TimeOverflowError,
        "oneflake.TimeOverflowError",
        ClockError
    ) < 0) {
        return -1;
    }
    if (create_exception(
        &LayoutDecodeError,
        "oneflake.LayoutDecodeError",
        DecodeError
    ) < 0) {
        return -1;
    }

    if (add_exception_to_module(
        module,
        "OneflakeException",
        OneflakeException
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "ConfigurationError",
        ConfigurationError
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "ClockError",
        ClockError
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "DecodeError",
        DecodeError
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "InvalidLayoutError",
        InvalidLayoutError
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "InvalidBitWidthError",
        InvalidBitWidthError
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "InvalidWorkerError",
        InvalidWorkerError
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "InvalidTimeUnitError",
        InvalidTimeUnitError
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "InvalidEpochError",
        InvalidEpochError
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "InvalidClockFunctionError",
        InvalidClockFunctionError
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "TimeBeforeEpochError",
        TimeBeforeEpochError
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "TimeReadError",
        TimeReadError
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "ClockRollbackError",
        ClockRollbackError
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "TimeOverflowError",
        TimeOverflowError
    ) < 0) {
        return -1;
    }
    if (add_exception_to_module(
        module,
        "LayoutDecodeError",
        LayoutDecodeError
    ) < 0) {
        return -1;
    }

    return 0;
}
