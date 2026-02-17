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
#include <stdint.h>
#include "_exceptions.h"
#include "_validation.h"
#include "_decoder.h"

PyTypeObject *FlakeDecoderType = NULL;
PyTypeObject *FlakePartsType = NULL;

static PyStructSequence_Field FlakeParts_fields[] = {
    { "timestamp", "Timestamp" },
    { "worker_id", "Worker ID" },
    { "sequence", "Sequence" },
    { NULL, NULL }
};

static PyStructSequence_Desc FlakeParts_desc = {
    "oneflake.FlakeParts",
    "Decoded oneflake ID.",
    FlakeParts_fields,
    3
};

int
FlakePartsType_init(void)
{
    if (FlakePartsType != NULL) {
        return 0;
    }
    FlakePartsType = PyStructSequence_NewType(&FlakeParts_desc);
    if (FlakePartsType == NULL) {
        return -1;
    }
    return 0;
}

/* Initialize FlakeDecoder instance */
static int
FlakeDecoder_init(FlakeDecoder *self, PyObject *args, PyObject *kwargs)
{
    int layout = (int)DEFAULT_LAYOUT;
    int timestamp_bits = (int)DEFAULT_TIMESTAMP_BITS;
    int worker_bits = (int)DEFAULT_WORKER_BITS;
    int sequence_bits = (int)DEFAULT_SEQUENCE_BITS;

    static const char *kwlist[] = {
        "layout",
        "timestamp_bits",
        "worker_bits",
        "sequence_bits",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(
        args,
        kwargs,
        "|$iiii",
        (char **)kwlist,
        &layout,
        &timestamp_bits,
        &worker_bits,
        &sequence_bits
    )) {
        return -1;
    }

    if (oneflake_validate_layout(layout) < 0) {
        return -1;
    }

    if (oneflake_validate_bit_width(
            timestamp_bits, worker_bits, sequence_bits
        ) < 0) {
        return -1;
    }

    self->layout = (OneflakeLayout)layout;
    self->timestamp_bits = (uint8_t)timestamp_bits;
    self->worker_bits = (uint8_t)worker_bits;
    self->sequence_bits = (uint8_t)sequence_bits;

    self->max_worker_id = (1ULL << self->worker_bits) - 1;
    self->max_sequence = (1ULL << self->sequence_bits) - 1;

    return 0;
}

/* Decode unique ID */
static PyObject *
FlakeDecoder_decode(FlakeDecoder *self, PyObject *args)
{
    PyObject *unique_id_obj;
    PyObject *decoded_id;
    PyObject *tmp_obj;
    uint64_t unique_id;
    uint64_t timestamp;
    uint64_t worker_id;
    uint64_t sequence;

    if (!PyArg_ParseTuple(args, "O", &unique_id_obj)) {
        return NULL;
    }

    unique_id = PyLong_AsUnsignedLongLong(unique_id_obj);
    if (unique_id == (uint64_t)-1 && PyErr_Occurred()) {
        return NULL;
    }

    if (self->layout == LAYOUT_TWS) {
        timestamp = unique_id >> (self->worker_bits + self->sequence_bits);
        worker_id = (unique_id >> self->sequence_bits) & self->max_worker_id;
        sequence = unique_id & self->max_sequence;
    }
    else if (self->layout == LAYOUT_TSW) {
        timestamp = unique_id >> (self->sequence_bits + self->worker_bits);
        sequence = (unique_id >> self->worker_bits) & self->max_sequence;
        worker_id = unique_id & self->max_worker_id;
    }
    else {
        PyErr_SetString(LayoutDecodeError, "Layout is not supported");
        return NULL;
    }

    decoded_id = PyStructSequence_New(FlakePartsType);
    if (decoded_id == NULL) {
        return NULL;
    }

    tmp_obj = PyLong_FromUnsignedLongLong(timestamp);
    if (tmp_obj == NULL) {
        Py_DECREF(decoded_id);
        return NULL;
    }
    PyStructSequence_SetItem(decoded_id, 0, tmp_obj);

    tmp_obj = PyLong_FromUnsignedLongLong(worker_id);
    if (tmp_obj == NULL) {
        Py_DECREF(decoded_id);
        return NULL;
    }
    PyStructSequence_SetItem(decoded_id, 1, tmp_obj);

    tmp_obj = PyLong_FromUnsignedLongLong(sequence);
    if (tmp_obj == NULL) {
        Py_DECREF(decoded_id);
        return NULL;
    }
    PyStructSequence_SetItem(decoded_id, 2, tmp_obj);

    return decoded_id;
}

/* Get timestamp bits */
static PyObject *
FlakeDecoder_get_timestamp_bits(FlakeDecoder *self, void *closure)
{
    (void)closure;
    return PyLong_FromUnsignedLong(self->timestamp_bits);
}

/* Get worker bits */
static PyObject *
FlakeDecoder_get_worker_bits(FlakeDecoder *self, void *closure)
{
    (void)closure;
    return PyLong_FromUnsignedLong(self->worker_bits);
}

/* Get sequence bits */
static PyObject *
FlakeDecoder_get_sequence_bits(FlakeDecoder *self, void *closure)
{
    (void)closure;
    return PyLong_FromUnsignedLong(self->sequence_bits);
}

/* Get layout configuration */
static PyObject *
FlakeDecoder_get_layout(FlakeDecoder *self, void *closure)
{
    (void)closure;
    return PyLong_FromLong(self->layout);
}

/* FlakeDecoder methods */
static PyMethodDef FlakeDecoder_methods[] = {
    {
        "decode",
        (PyCFunction)FlakeDecoder_decode,
        METH_VARARGS,
        "Decode unique ID"
    },
    {
        NULL,
        NULL,
        0,
        NULL
    }
};

/* FlakeDecoder getters and setters */
static PyGetSetDef FlakeDecoder_getset[] = {
    {
        "timestamp_bits",
        (getter)FlakeDecoder_get_timestamp_bits,
        NULL,
        "Timestamp bits",
        NULL
    },
    {
        "worker_bits",
        (getter)FlakeDecoder_get_worker_bits,
        NULL,
        "Worker bits",
        NULL
    },
    {
        "sequence_bits",
        (getter)FlakeDecoder_get_sequence_bits,
        NULL,
        "Sequence bits",
        NULL
    },
    {
        "layout",
        (getter)FlakeDecoder_get_layout,
        NULL,
        "Layout",
        NULL
    },
    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

static PyType_Slot FlakeDecoder_slots[] = {
    {Py_tp_doc, (void *)"Flake decoder."},
    {Py_tp_methods, (void *)FlakeDecoder_methods},
    {Py_tp_getset, (void *)FlakeDecoder_getset},
    {Py_tp_init, (void *)FlakeDecoder_init},
    {Py_tp_new, (void *)PyType_GenericNew},
    {0, NULL}
};

static PyType_Spec FlakeDecoder_spec = {
    .name = "oneflake.FlakeDecoder",
    .basicsize = sizeof(FlakeDecoder),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = FlakeDecoder_slots
};

/* Initialize Oneflake decoder module */
int
decoder_init(PyObject *module)
{
    if (FlakePartsType_init() < 0) {
        return -1;
    }
    if (FlakeDecoderType == NULL) {
        FlakeDecoderType = (PyTypeObject *)PyType_FromSpec(&FlakeDecoder_spec);
        if (FlakeDecoderType == NULL) {
            return -1;
        }
    }

    Py_INCREF((PyObject *)FlakeDecoderType);
    if (PyModule_AddObject(
        module, "FlakeDecoder", (PyObject *)FlakeDecoderType
    ) < 0) {
        Py_DECREF((PyObject *)FlakeDecoderType);
        return -1;
    }

    Py_INCREF(FlakePartsType);
    if (PyModule_AddObject(
        module, "FlakeParts", (PyObject *)FlakePartsType
    ) < 0) {
        Py_DECREF(FlakePartsType);
        return -1;
    }

    return 0;
}
