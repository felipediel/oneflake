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

#ifndef ONEFLAKE_CONSTANTS_H
#define ONEFLAKE_CONSTANTS_H

#include <Python.h>
#include <limits.h>

typedef enum {
    LAYOUT_TWS = 0,
    LAYOUT_TSW = 1
} OneflakeLayout;

#define UNIX_EPOCH_MS 0ULL
#define MIN_EPOCH_MS UNIX_EPOCH_MS
#define MAX_EPOCH_MS (UINT64_MAX / 1000000ULL)

#define DEFAULT_WORKER_ID 0
#define DEFAULT_LAYOUT LAYOUT_TWS
#define DEFAULT_TIMESTAMP_BITS 41
#define DEFAULT_WORKER_BITS 10
#define DEFAULT_SEQUENCE_BITS 12
#define DEFAULT_TIME_UNIT_NS 1000000ULL
#define DEFAULT_EPOCH_MS 1288834974657ULL

int constants_init(PyObject *module);

#endif /* ONEFLAKE_CONSTANTS_H */
