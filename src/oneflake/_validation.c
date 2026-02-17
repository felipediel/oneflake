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

#include "_constants.h"
#include "_exceptions.h"
#include "_validation.h"

static int
oneflake_validate_bit_width_range(int bits, const char *error_message)
{
    if (bits < 1 || bits > 61) {
        PyErr_SetString(InvalidBitWidthError, error_message);
        return -1;
    }

    return 0;
}

int
oneflake_validate_layout(int layout)
{
    if (layout != LAYOUT_TWS && layout != LAYOUT_TSW) {
        PyErr_SetString(InvalidLayoutError, ONEFLAKE_ERR_INVALID_LAYOUT);
        return -1;
    }

    return 0;
}

int
oneflake_validate_bit_width(
    int timestamp_bits,
    int worker_bits,
    int sequence_bits
)
{
    if (oneflake_validate_bit_width_range(
        timestamp_bits, ONEFLAKE_ERR_TIMESTAMP_BITS_RANGE
    ) < 0) {
        return -1;
    }

    if (oneflake_validate_bit_width_range(
        worker_bits, ONEFLAKE_ERR_WORKER_BITS_RANGE
    ) < 0) {
        return -1;
    }

    if (oneflake_validate_bit_width_range(
        sequence_bits, ONEFLAKE_ERR_SEQUENCE_BITS_RANGE
    ) < 0) {
        return -1;
    }

    if (timestamp_bits + worker_bits + sequence_bits != 63) {
        PyErr_SetString(InvalidBitWidthError, ONEFLAKE_ERR_BITS_SUM);
        return -1;
    }

    return 0;
}

int
oneflake_validate_worker_id(long long worker_id, int worker_bits)
{
    unsigned long long max_worker_id = (1ULL << worker_bits) - 1;

    if (worker_id < 0 || (unsigned long long)worker_id > max_worker_id) {
        PyErr_Format(
            InvalidWorkerError,
            ONEFLAKE_ERR_WORKER_ID_RANGE,
            max_worker_id
        );
        return -1;
    }

    return 0;
}

int
oneflake_validate_time_unit_ns(long long time_unit_ns, int timestamp_bits)
{
    unsigned long long max_time_unit_ns;

    max_time_unit_ns = (1ULL << timestamp_bits) - 1;
    if (
        time_unit_ns < 1
        || (unsigned long long)time_unit_ns > max_time_unit_ns
    ) {
        PyErr_Format(
            InvalidTimeUnitError,
            ONEFLAKE_ERR_TIME_UNIT_NS_RANGE,
            max_time_unit_ns
        );
        return -1;
    }

    return 0;
}

int
oneflake_validate_epoch_ms(long long epoch_ms)
{
    if (epoch_ms < 0 || (unsigned long long)epoch_ms > MAX_EPOCH_MS) {
        PyErr_Format(
            InvalidEpochError, ONEFLAKE_ERR_EPOCH_MS_RANGE, MAX_EPOCH_MS
        );
        return -1;
    }

    return 0;
}

int
oneflake_validate_clock_func(PyObject *clock_func)
{
    if (clock_func != Py_None && !PyCallable_Check(clock_func)) {
        PyErr_SetString(
            InvalidClockFunctionError,
            ONEFLAKE_ERR_CLOCK_FUNC_CALLABLE_OR_NONE
        );
        return -1;
    }

    return 0;
}
