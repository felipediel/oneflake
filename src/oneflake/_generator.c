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
#include <time.h>
#include "_exceptions.h"
#include "_validation.h"
#include "_generator.h"

PyTypeObject *FlakeGeneratorType = NULL;

/* Get timestamp in nanoseconds from system clock */
static inline int8_t
get_timestamp_from_system_clock(uint64_t *timestamp_ptr)
{
    struct timespec ts;

    if (timespec_get(&ts, TIME_UTC) != TIME_UTC) {
        return -1;
    }
    *timestamp_ptr = (
        (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec
    );
    return 0;
}

/* Get timestamp in nanoseconds from callable */
static int8_t
get_timestamp_from_callable(PyObject *clock_func, uint64_t *timestamp_ptr)
{
    uint64_t timestamp = 0;
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject *timestamp_obj;

    timestamp_obj = PyObject_CallObject(clock_func, NULL);
    if (timestamp_obj != NULL) {
        timestamp = PyLong_AsUnsignedLongLong(timestamp_obj);
        Py_DECREF(timestamp_obj);
    }

    PyGILState_Release(gstate);
    if (PyErr_Occurred()) {
        return -1;
    }

    *timestamp_ptr = timestamp;
    return 0;
}

/* Encode timestamp and sequence into a state value */
static inline uint64_t
encode_state(uint64_t timestamp, uint64_t sequence, uint8_t sequence_bits)
{
    return (timestamp << sequence_bits) | sequence;
}

/* Extract timestamp from state */
static inline uint64_t
extract_timestamp_from_state(uint64_t state, uint8_t sequence_bits)
{
    return state >> sequence_bits;
}

/* Extract sequence from state */
static inline uint64_t
extract_sequence_from_state(uint64_t state, uint64_t max_sequence)
{
    return state & max_sequence;
}

/* Get timestamp relative to epoch in time units */
static inline int8_t
get_timestamp(FlakeGenerator *self, uint64_t *timestamp_ptr)
{
    uint64_t timestamp;
    int8_t ret;

    if (self->clock_func != NULL) {
        ret = get_timestamp_from_callable(self->clock_func, &timestamp);
    }
    else {
        ret = get_timestamp_from_system_clock(&timestamp);
    }

    if (ret != 0) {
        return -1;
    }

    timestamp /= self->time_unit_ns;
    if (timestamp < self->epoch) {
        PyErr_SetString(
            TimeBeforeEpochError,
            "Current timestamp is before the configured epoch"
        );
        return -1;
    }

    *timestamp_ptr = timestamp - self->epoch;
    return 0;
}

/* Wait for the next timestamp */
static inline int8_t
wait_for_next_timestamp(
    FlakeGenerator *self,
    uint64_t prev_ts,
    uint64_t *timestamp_ptr
)
{
    uint64_t timestamp;

    do {
        if (get_timestamp(self, &timestamp) != 0) {
            return -1;
        }
        if (timestamp > self->max_timestamp) {
            PyErr_SetString(
                TimeOverflowError,
                "Timestamp exceeded the maximum allowed value"
            );
            return -1;
        }
    } while (timestamp <= prev_ts);

    *timestamp_ptr = timestamp;
    return 0;
}

/* Deallocate FlakeGenerator instance */
static void
FlakeGenerator_dealloc(FlakeGenerator *self)
{
    freefunc tp_free;

    Py_XDECREF(self->clock_func);
    tp_free = (freefunc)PyType_GetSlot(Py_TYPE(self), Py_tp_free);
    if (tp_free != NULL) {
        tp_free((PyObject *)self);
    }
    else {
        PyObject_Free((void *)self);
    }
}

/* Initialize FlakeGenerator instance */
static int
FlakeGenerator_init(FlakeGenerator *self, PyObject *args, PyObject *kwargs)
{
    long long worker_id = (long long)DEFAULT_WORKER_ID;
    int layout = (int)DEFAULT_LAYOUT;
    int timestamp_bits = (int)DEFAULT_TIMESTAMP_BITS;
    int worker_bits = (int)DEFAULT_WORKER_BITS;
    int sequence_bits = (int)DEFAULT_SEQUENCE_BITS;
    long long time_unit_ns = (long long)DEFAULT_TIME_UNIT_NS;
    long long epoch_ms = (long long)DEFAULT_EPOCH_MS;
    PyObject *clock_func = Py_None;

    static const char *kwlist[] = {
        "worker_id",
        "layout",
        "timestamp_bits",
        "worker_bits",
        "sequence_bits",
        "time_unit_ns",
        "epoch_ms",
        "clock_func",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(
        args,
        kwargs,
        "|L$iiiiLLO",
        (char **)kwlist,
        &worker_id,
        &layout,
        &timestamp_bits,
        &worker_bits,
        &sequence_bits,
        &time_unit_ns,
        &epoch_ms,
        &clock_func
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

    if (oneflake_validate_worker_id(worker_id, worker_bits) < 0) {
        return -1;
    }

    if (oneflake_validate_time_unit_ns(time_unit_ns, timestamp_bits) < 0) {
        return -1;
    }

    if (oneflake_validate_epoch_ms(epoch_ms) < 0) {
        return -1;
    }

    if (oneflake_validate_clock_func(clock_func) < 0) {
        return -1;
    }

    self->worker_id = (uint64_t)worker_id;
    self->layout = (OneflakeLayout)layout;
    self->timestamp_bits = (uint8_t)timestamp_bits;
    self->worker_bits = (uint8_t)worker_bits;
    self->sequence_bits = (uint8_t)sequence_bits;
    self->time_unit_ns = (uint64_t)time_unit_ns;
    self->epoch = ((uint64_t)epoch_ms * 1000000ULL) / self->time_unit_ns;

    if (clock_func == Py_None) {
        self->clock_func = NULL;
    }
    else {
        Py_INCREF(clock_func);
        self->clock_func = clock_func;
    }

    self->max_timestamp = (1ULL << self->timestamp_bits) - 1;
    self->max_worker_id = (1ULL << self->worker_bits) - 1;
    self->max_sequence = (1ULL << self->sequence_bits) - 1;
    self->timestamp_shift = (uint8_t)(self->worker_bits + self->sequence_bits);

    if (self->layout == LAYOUT_TWS) {
        self->worker_shift = self->sequence_bits;
        self->sequence_shift = 0;
    }
    else {
        self->worker_shift = 0;
        self->sequence_shift = self->worker_bits;
    }

    atomic_init(&self->state, 0);

    return 0;
}

/* Generate unique ID */
static PyObject *
FlakeGenerator_generate(FlakeGenerator *self, PyObject *Py_UNUSED(ignored))
{
    const uint32_t max_clock_catchup_attempts = (1U << 14);
    uint64_t timestamp;
    uint64_t state;
    uint64_t desired_state;
    uint64_t unique_id;
    uint32_t clock_catchup_attempts = 0;

    uint64_t prev_ts;
    uint64_t prev_seq;
    uint64_t new_seq;

    while (1) {
        if (get_timestamp(self, &timestamp) != 0) {
            set_exception_from_cause(
                TimeReadError, "Failed to retrieve timestamp"
            );
            return NULL;
        }
        if (timestamp > self->max_timestamp) {
            PyErr_SetString(
                TimeOverflowError,
                "Timestamp exceeded the maximum allowed value"
            );
            return NULL;
        }

        state = atomic_load_explicit(&self->state, memory_order_relaxed);
        prev_ts = extract_timestamp_from_state(state, self->sequence_bits);
        prev_seq = extract_sequence_from_state(state, self->max_sequence);

        if (timestamp < prev_ts) {
            clock_catchup_attempts++;
            if (clock_catchup_attempts >= max_clock_catchup_attempts) {
                PyErr_SetString(ClockRollbackError, "Clock moved backwards");
                return NULL;
            }
            continue;
        }
        clock_catchup_attempts = 0;

        if (timestamp == prev_ts) {
            new_seq = prev_seq + 1;
            if (new_seq > self->max_sequence) {
                if (wait_for_next_timestamp(self, prev_ts, &timestamp) != 0) {
                    set_exception_from_cause(
                        TimeReadError, "Failed to retrieve timestamp"
                    );
                    return NULL;
                }
                new_seq = 0;
            }
        } else {
            new_seq = 0;
        }

        desired_state = encode_state(timestamp, new_seq, self->sequence_bits);
        if (atomic_compare_exchange_weak_explicit(
            &self->state,
            &state,
            desired_state,
            memory_order_relaxed,
            memory_order_relaxed
        )) {
            break;
        }
    }

    unique_id = (
        (timestamp << self->timestamp_shift)
        | (self->worker_id << self->worker_shift)
        | (new_seq << self->sequence_shift)
    );

    return PyLong_FromUnsignedLongLong(unique_id);
}

/* Get worker ID */
static PyObject *
FlakeGenerator_get_worker_id(FlakeGenerator *self, void *closure)
{
    (void)closure;
    return PyLong_FromUnsignedLongLong(self->worker_id);
}

/* Get epoch */
static PyObject *
FlakeGenerator_get_epoch(FlakeGenerator *self, void *closure)
{
    (void)closure;
    return PyLong_FromUnsignedLongLong(self->epoch);
}

/* Get timestamp bits */
static PyObject *
FlakeGenerator_get_timestamp_bits(FlakeGenerator *self, void *closure)
{
    (void)closure;
    return PyLong_FromUnsignedLong(self->timestamp_bits);
}

/* Get worker bits */
static PyObject *
FlakeGenerator_get_worker_bits(FlakeGenerator *self, void *closure)
{
    (void)closure;
    return PyLong_FromUnsignedLong(self->worker_bits);
}

/* Get sequence bits */
static PyObject *
FlakeGenerator_get_sequence_bits(FlakeGenerator *self, void *closure)
{
    (void)closure;
    return PyLong_FromUnsignedLong(self->sequence_bits);
}

/* Get time unit in nanoseconds */
static PyObject *
FlakeGenerator_get_time_unit_ns(FlakeGenerator *self, void *closure)
{
    (void)closure;
    return PyLong_FromUnsignedLongLong(self->time_unit_ns);
}

/* Get layout configuration */
static PyObject *
FlakeGenerator_get_layout(FlakeGenerator *self, void *closure)
{
    (void)closure;
    return PyLong_FromLong(self->layout);
}

/* FlakeGenerator methods */
static PyMethodDef FlakeGenerator_methods[] = {
    {
        "generate",
        (PyCFunction)FlakeGenerator_generate,
        METH_NOARGS,
        "Generate unique ID"
    },
    {
        NULL,
        NULL,
        0,
        NULL
    }
};

/* FlakeGenerator getters and setters */
static PyGetSetDef FlakeGenerator_getset[] = {
    {
        "worker_id",
        (getter)FlakeGenerator_get_worker_id,
        NULL,
        "Worker ID",
        NULL
    },
    {
        "epoch",
        (getter)FlakeGenerator_get_epoch,
        NULL,
        "Epoch",
        NULL
    },
    {
        "timestamp_bits",
        (getter)FlakeGenerator_get_timestamp_bits,
        NULL,
        "Timestamp bits",
        NULL
    },
    {
        "worker_bits",
        (getter)FlakeGenerator_get_worker_bits,
        NULL,
        "Worker bits",
        NULL
    },
    {
        "sequence_bits",
        (getter)FlakeGenerator_get_sequence_bits,
        NULL,
        "Sequence bits",
        NULL
    },
    {
        "time_unit_ns",
        (getter)FlakeGenerator_get_time_unit_ns,
        NULL,
        "Time unit in nanoseconds",
        NULL
    },
    {
        "layout",
        (getter)FlakeGenerator_get_layout,
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

static PyType_Slot FlakeGenerator_slots[] = {
    {Py_tp_doc, (void *)"Flake generator."},
    {Py_tp_methods, (void *)FlakeGenerator_methods},
    {Py_tp_getset, (void *)FlakeGenerator_getset},
    {Py_tp_init, (void *)FlakeGenerator_init},
    {Py_tp_dealloc, (void *)FlakeGenerator_dealloc},
    {Py_tp_new, (void *)PyType_GenericNew},
    {0, NULL}
};

static PyType_Spec FlakeGenerator_spec = {
    .name = "oneflake.FlakeGenerator",
    .basicsize = sizeof(FlakeGenerator),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = FlakeGenerator_slots
};

/* Initialize generator module */
int
generator_init(PyObject *module)
{
    if (FlakeGeneratorType == NULL) {
        FlakeGeneratorType = (PyTypeObject *)PyType_FromSpec(&FlakeGenerator_spec);
        if (FlakeGeneratorType == NULL) {
            return -1;
        }
    }

    Py_INCREF((PyObject *)FlakeGeneratorType);
    if (PyModule_AddObject(
        module, "FlakeGenerator", (PyObject *)FlakeGeneratorType
    ) < 0) {
        Py_DECREF((PyObject *)FlakeGeneratorType);
        return -1;
    }

    return 0;
}
