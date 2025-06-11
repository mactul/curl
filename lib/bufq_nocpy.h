/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/

#ifndef BUFQ_NOCPY_H
#define BUFQ_NOCPY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum BUFQ_ALLOC_METHOD {
    BUFQ_ALLOC_STATIC,
    BUFQ_ALLOC_MALLOC,
    BUFQ_ALLOC_EXTERNAL
};

typedef struct bufq_nocpy BufqNoCpy;

BufqNoCpy *Curl_bufq_nocpy_init(size_t nb_buffers_base);

bool Curl_bufq_nocpy_write(BufqNoCpy *queue, unsigned char *data,
                           size_t size, enum BUFQ_ALLOC_METHOD alloc_method);

size_t Curl_bufq_nocpy_read(BufqNoCpy *queue, unsigned char **data);

size_t Curl_bufq_nocpy_read_cpy(BufqNoCpy *queue, unsigned char *buf,
                                size_t bufsize, unsigned char **ptr_to_free,
                                enum BUFQ_ALLOC_METHOD *alloc_method);

void Curl_bufq_nocpy_free(BufqNoCpy **pqueue, void (*free_func)(void *));

bool Curl_bufq_nocpy_is_empty(BufqNoCpy *queue);

#endif