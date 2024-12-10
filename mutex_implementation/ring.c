/*
 *
 *  Producer-Consumer Lab
 *
 *  Copyright (c) 2022 Peter A. Dinda, Branden Ghena
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atomics.h"
#include "config.h"
#include "ring.h"


ring_t* ring_create(uint32_t size,
                    uint32_t producer_limit,
                    int      producer_interrupts,
                    uint32_t consumer_limit,
                    int      consumer_interrupts) {
  // the handout version does not observe the limits or interrupts

  uint64_t numbytes = sizeof(ring_t) + (size * sizeof(void*));
  ring_t* ring      = (ring_t*)malloc(numbytes);

  if (!ring) {
    ERROR("Cannot allocate\n");
    return 0;
  }

  memset(ring, 0, numbytes);

  DEBUG("allocation is at %p, data at %p\n", ring, ring->elements);

  ring->size = size;
  ring->head = 0;
  ring->tail = 0;

  pthread_mutex_init(&ring->mutex, NULL);
  pthread_cond_init(&ring->push, NULL);
  pthread_cond_init(&ring->pull, NULL);

  DEBUG("ring %p created\n", ring);

  // WRITE ME!
  //
  // any synchronization state would be initialized here

  return ring;
}

void ring_destroy(ring_t* ring) {
  DEBUG("destroy %p\n", ring);
  // do we need to wait for requests to finish?
  free(ring);
}


// Helper Functions

// note that this is NOT synchronized and so prone to races
static bool can_push(ring_t* ring) {
  return (ring->head - ring->tail) < ring->size;
}

// note that this is NOT synchronized and so prone to races!
static bool can_pull(ring_t* ring) {
  return ring->tail < ring->head;
}


// Producer Side

// note that this is NOT synchronized and so prone to races!
int ring_push(ring_t* ring, void* data) {
  pthread_mutex_lock(&ring->mutex);
  while (!can_push(ring)){
    pthread_cond_wait(&ring->push, &ring->mutex);
  }

  DEBUG("starting push of %p to %p\n", data, ring);

  ring->elements[ring->head % ring->size] = data;
  ring->head++;

  DEBUG("push done\n");

  pthread_cond_signal(&ring->pull);
  pthread_mutex_unlock(&ring->mutex);

  return 0;
}

// note that this is NOT synchronized and so prone to races!
int ring_try_push(ring_t* ring, void* data) {

  if (pthread_mutex_trylock(&ring->mutex) != 0){
    return 1;
  }

  if (!can_push(ring)) {
    pthread_mutex_unlock(&ring->mutex);
    return 1;
  }

  ring->elements[ring->head % ring->size] = data;
  ring->head++;


  pthread_cond_signal(&ring->pull);
  pthread_mutex_unlock(&ring->mutex);

  return 0;
}


// Consumer Side

// note that this is NOT synchronized and so prone to races!
int ring_pull(ring_t* ring, void** data) {
  pthread_mutex_lock(&ring->mutex);
  while (!can_pull(ring)){
    pthread_cond_wait(&ring->pull, &ring->mutex);
  }

  DEBUG("starting pull from %p\n", ring);

  *data = ring->elements[ring->tail % ring->size];
  ring->tail++;

  DEBUG("pulled %p\n", *data);

  pthread_cond_signal(&ring->push);
  pthread_mutex_unlock(&ring->mutex);

  return 0;
}

// note that this is NOT synchronized and so prone to races!
int ring_try_pull(ring_t* ring, void** data) {

  if (pthread_mutex_trylock(&ring->mutex) != 0){
    return 1;
  }

  if (!can_pull(ring)) {
    pthread_mutex_unlock(&ring->mutex);
    return 1;
  }

  *data = ring->elements[ring->tail % ring->size];
  ring->tail++;

  pthread_cond_signal(&ring->push);
  pthread_mutex_unlock(&ring->mutex);

  return 0;
}
