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

#ifndef ONEFLAKE_VALIDATION_H
#define ONEFLAKE_VALIDATION_H

#include <Python.h>

#define ONEFLAKE_ERR_INVALID_LAYOUT "Layout must be 0 (TWS) or 1 (TSW)"
#define ONEFLAKE_ERR_TIMESTAMP_BITS_RANGE "Timestamp bits must be between 1 and 61"
#define ONEFLAKE_ERR_WORKER_BITS_RANGE "Worker bits must be between 1 and 61"
#define ONEFLAKE_ERR_SEQUENCE_BITS_RANGE "Sequence bits must be between 1 and 61"
#define ONEFLAKE_ERR_BITS_SUM "The sum of timestamp bits, worker bits, and sequence bits must be 63"
#define ONEFLAKE_ERR_WORKER_ID_RANGE "Worker ID must be between 0 and %llu"
#define ONEFLAKE_ERR_TIME_UNIT_NS_RANGE "Time unit must be between 1 and %llu"
#define ONEFLAKE_ERR_EPOCH_MS_RANGE "Epoch must be between 0 and %llu"
#define ONEFLAKE_ERR_CLOCK_FUNC_CALLABLE_OR_NONE "The clock_func argument must be a callable object or None"

int oneflake_validate_layout(int layout);

int oneflake_validate_bit_width(
    int timestamp_bits,
    int worker_bits,
    int sequence_bits
);

int oneflake_validate_worker_id(long long worker_id, int worker_bits);

int oneflake_validate_time_unit_ns(long long time_unit_ns, int timestamp_bits);

int oneflake_validate_epoch_ms(long long epoch_ms);

int oneflake_validate_clock_func(PyObject *clock_func);

#endif /* ONEFLAKE_VALIDATION_H */
