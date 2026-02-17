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

#ifndef ONEFLAKE_GENERATOR_H
#define ONEFLAKE_GENERATOR_H

#include <Python.h>
#include <stdatomic.h>
#include <stdint.h>
#include "_constants.h"

typedef struct {
    PyObject_HEAD
    uint64_t worker_id;
    OneflakeLayout layout;
    uint8_t timestamp_bits;
    uint8_t worker_bits;
    uint8_t sequence_bits;
    uint64_t time_unit_ns;
    uint64_t epoch;
    PyObject *clock_func;
    uint64_t max_timestamp;
    uint64_t max_worker_id;
    uint64_t max_sequence;
    uint8_t timestamp_shift;
    uint8_t worker_shift;
    uint8_t sequence_shift;
    atomic_uint_fast64_t state;
} FlakeGenerator;

extern PyTypeObject *FlakeGeneratorType;

int generator_init(PyObject *module);

#endif /* ONEFLAKE_GENERATOR_H */
