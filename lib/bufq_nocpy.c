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

#include "bufq_nocpy.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct bufq_nocpy_chunk
{
  enum BUFQ_ALLOC_METHOD alloc_method;
  size_t size;
  size_t read_offset;
  unsigned char *data;
};

struct bufq_nocpy
{
  size_t nb_buffers;
  ssize_t input_index;
  ssize_t output_index;
  struct bufq_nocpy *next;
  struct bufq_nocpy_chunk chunks[];
};

BufqNoCpy *Curl_bufq_nocpy_init(size_t nb_buffers_base)
{
  BufqNoCpy *queue;

  if(nb_buffers_base < 1) {
    return NULL;
  }
  queue = (BufqNoCpy *)calloc(1, sizeof(BufqNoCpy)
                                 + nb_buffers_base
                                 * sizeof(struct bufq_nocpy_chunk));
  if(!queue) {
    return NULL;
  }
  queue->nb_buffers = nb_buffers_base;
  queue->output_index = -1;

  return queue;
}

bool Curl_bufq_nocpy_write(BufqNoCpy *queue, unsigned char *data,
                           size_t size, enum BUFQ_ALLOC_METHOD alloc_method)
{
  if(size == 0) {
    /* If we add an empty buffer, the queue can appear to be empty when reading
       it It's also work done for nothing */
    return true;
  }
  while(queue->next) {
    queue = queue->next;
  }

  if(queue->input_index == queue->output_index) {
    queue->next = Curl_bufq_nocpy_init(queue->nb_buffers);
    if(!queue->next) {
      return false; /* out of memory */
    }
    queue = queue->next;
  }
  queue->chunks[queue->input_index].size = size;
  queue->chunks[queue->input_index].data = data;
  queue->chunks[queue->input_index].read_offset = 0;
  queue->chunks[queue->input_index].alloc_method = alloc_method;
  if(queue->output_index == -1) {
    queue->output_index = queue->input_index;
  }
  queue->input_index = (queue->input_index + 1) % (ssize_t)queue->nb_buffers;
  return true;
}

size_t Curl_bufq_nocpy_read(BufqNoCpy *queue, unsigned char **data)
{
  size_t size;
  BufqNoCpy *parent = NULL;

  while(queue->output_index == -1) {
    if(!queue->next) {
      /* queue is empty */
      *data = NULL;
      return 0;
    }
    parent = queue;
    queue = queue->next;
  }

  *data = queue->chunks[queue->output_index].data;
  size = queue->chunks[queue->output_index].size;

  /* to be able to free dangling pointers */
  queue->chunks[queue->output_index].data = NULL;
  queue->chunks[queue->output_index].size = 0;

  queue->output_index = (queue->output_index + 1) % (ssize_t)queue->nb_buffers;
  if(queue->output_index == queue->input_index) {
    /* this queue is now empty */
    queue->output_index = -1;
    if(!queue->next && parent) {
      /* it's a tail queue, we can destroy it
         it will take less memory but it will also
         be faster */
      free(queue);
      parent->next = NULL;
    }
  }

  return size;
}

size_t Curl_bufq_nocpy_read_cpy(BufqNoCpy *queue, unsigned char *buf,
                           size_t bufsize, unsigned char **ptr_to_free,
                           size_t *ptr_to_free_size,
                           enum BUFQ_ALLOC_METHOD *alloc_method)
{
  size_t size;
  size_t left;
  BufqNoCpy *parent = NULL;

  *ptr_to_free = NULL;
  *ptr_to_free_size = 0;

  while(queue->output_index == -1) {
    if(!queue->next) {
      /* queue is empty */
      return 0;
    }
    parent = queue;
    queue = queue->next;
  }

  *alloc_method = queue->chunks[queue->output_index].alloc_method;

  left = queue->chunks[queue->output_index].size
         - queue->chunks[queue->output_index].read_offset;

  size = MIN(bufsize, left);
  memcpy(buf,
         queue->chunks[queue->output_index].data
         + queue->chunks[queue->output_index].read_offset, size);

  if(size < left) {
    /* The buffer hasn't been entirely consumed */
    queue->chunks[queue->output_index].read_offset += size;
    return size;
  }

  /* The buffer hasn't been entirely consumed, it's time to free it */
  if(queue->chunks[queue->output_index].alloc_method != BUFQ_ALLOC_STATIC) {
    *ptr_to_free = queue->chunks[queue->output_index].data;
    *ptr_to_free_size = queue->chunks[queue->output_index].size;
  }

  queue->chunks[queue->output_index].data = NULL;
  queue->chunks[queue->output_index].size = 0;
  queue->chunks[queue->output_index].read_offset = 0;

  queue->output_index = (queue->output_index + 1) % (ssize_t)queue->nb_buffers;
  if(queue->output_index == queue->input_index) {
    /* This queue is now empty */
    queue->output_index = -1;
    if(!queue->next && parent) {
      /* it's a tail queue, we can destroy it
         it will take less memory but it will also
         be faster */
      free(queue);
      parent->next = NULL;
    }
  }

  return size;
}

bool Curl_bufq_nocpy_is_empty(BufqNoCpy *queue)
{
  while(queue->output_index == -1) {
    if(!queue->next) {
      /* queue is empty */
      return true;
    }
    queue = queue->next;
  }
  return false;
}

static void rec_free(BufqNoCpy *queue, void (*free_func)(void *))
{
  if(queue->next) {
    rec_free(queue->next, free_func);
  }
  if(free_func) {
    for(size_t i = 0; i < queue->nb_buffers; i++) {
      if(queue->chunks[i].alloc_method == BUFQ_ALLOC_MALLOC) {
        free_func(queue->chunks[i].data);
      }
    }
  }
  free(queue);
}

void Curl_bufq_nocpy_free(BufqNoCpy **pqueue, void (*free_func)(void *))
{
  if(!*pqueue) {
    return;
  }
  rec_free(*pqueue, free_func);
  *pqueue = NULL;
}