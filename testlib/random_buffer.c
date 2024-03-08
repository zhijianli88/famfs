/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2024 Micron Technology, Inc.  All rights reserved.
 */

#include <string.h>
#include <assert.h>

#include "xrand.h"

void
randomize_buffer(void *buf, size_t len, unsigned int seed)
{
    unsigned int *  tmp = (unsigned int *)buf;
    u_int           last;
    uint64_t        remain = len;
    uint64_t        i;
    struct xrand xr;

    if (len == 0)
        return;

    xrand_init(&xr, seed);
    for (i = 0; remain > 0; i++, remain -= sizeof(*tmp)) {
        if (remain > sizeof(*tmp)) { /* likely */
            tmp[i] = xrand64(&xr);
        } else { /* unlikely */
            last = xrand64(&xr);
            memcpy(&tmp[i], &last, remain);
        }
    }
}

int64_t
validate_random_buffer(void *buf, size_t len, unsigned int seed)
{
    unsigned int *  tmp = (unsigned int *)buf;
    unsigned int    val;
    char *          expect = (char *)&val;
    char *          found;
    uint64_t        remain = len;
    uint64_t        i;
    struct xrand xr;

    if (len == 0)
        return -1; /* success... */

    xrand_init(&xr, seed);
    for (i = 0; remain > 0; i++, remain -= sizeof(*tmp)) {
        val = xrand64(&xr);
        if ((remain >= sizeof(*tmp)) && (val != tmp[i])) { /* Likely */
            return ((int64_t)(len - remain));
        } else if (remain < sizeof(*tmp)) { /* Unlikely */
            found = (char *)&tmp[i];
            if (memcmp(expect, found, remain)) {
                /*
                 * [HSE_REVISIT]
                 * Miscompare offset might be off here
                 */
                return ((int64_t)(len - remain));
            }
        }
    }
    /* -1 is success, because 0..n are valid offsets for an error */
    return -1;
}

/* Get a random value in the range [min, max]. Note that the max is an inclusive upper bound.
 */
u_int32_t
generate_random_u32(u_int32_t min, u_int32_t max)
{
    return (xrand64_tls() % (max - min + 1)) + min;
}

void
permute_u32_sequence(u_int32_t *values, u_int32_t num_values)
{
    u_int32_t i, j, tmp_val;

    for (i = num_values - 1; i > 0; --i) {
        j = generate_random_u32(0, i - 1);
        tmp_val = values[i];
        values[i] = values[j];
        values[j] = tmp_val;
    }
}

void
generate_random_u32_sequence(u_int32_t min_value, u_int32_t max_value, u_int32_t *values, u_int32_t num_values)
{
    u_int32_t i;

    for (i = 0; i < num_values; ++i)
        values[i] = generate_random_u32(min_value, max_value);

    permute_u32_sequence(values, num_values);
}

void
generate_random_u32_sequence_unique(u_int32_t min_value, u_int32_t max_value, u_int32_t *values, u_int32_t num_values)
{
    u_int32_t i;
    u_int32_t stride = (max_value - min_value) / num_values;

    assert(stride > 0);

    for (i = 0; i < num_values; ++i) {
        u_int32_t min = i * stride;
        u_int32_t max = min + stride - 1;

        if (i == num_values - 1)
            max = max_value;

        values[i] = generate_random_u32(min, max);
    }

    permute_u32_sequence(values, num_values);
}
