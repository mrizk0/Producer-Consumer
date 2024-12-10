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

  DEBUG("ring %p created\n", ring);

  // WRITE ME!
  //
  // any synchronization state would be initialized here

  ring -> lock = 0;

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
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT

  // try to acquire lock
  while (atomic_exchange(&(ring->lock), 1) == 1){
    // pause();
  }
  // if we can't pull
  while (!can_push(ring)){

    // let go of lock
    atomic_store(&(ring->lock), 0);

    // try to acquire again to check can_pull status
    while (atomic_exchange(&(ring->lock), 1) == 1);

  }


  DEBUG("starting push of %p to %p\n", data, ring);

  ring->elements[ring->head % ring->size] = data;
  ring->head++;

  DEBUG("push done\n");

  // let go of lock
  atomic_store(&(ring->lock), 0);

  return 0;
}

// note that this is NOT synchronized and so prone to races!
int ring_try_push(ring_t* ring, void* data) {
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT

  // if we can't get the lock
  if (atomic_exchange(&(ring->lock), 1) == 1){
    // exit
    return 1;
  }

  // if we can't push
  if (!can_push(ring)) {
    // exit
    atomic_store(&(ring->lock), 0);
    return 1;
  }

  ring->elements[ring->head % ring->size] = data;
  ring->head++;

  atomic_store(&(ring->lock), 0);

  return 0;
}


// Consumer Side

// note that this is NOT synchronized and so prone to races!
int ring_pull(ring_t* ring, void** data) {
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT

  while (atomic_exchange(&(ring->lock), 1) == 1);


  // if we can't pull
  while (!can_pull(ring)){

    // let go of lock
    atomic_store(&(ring->lock), 0);

    // try to acquire again to check can_pull status
    while (atomic_exchange(&(ring->lock), 1) == 1);
  }

  DEBUG("starting pull from %p\n", ring);

  *data = ring->elements[ring->tail % ring->size];
  ring->tail++;

  DEBUG("pulled %p\n", *data);

  atomic_store(&(ring->lock), 0);

  return 0;
}

// note that this is NOT synchronized and so prone to races!
int ring_try_pull(ring_t* ring, void** data) {
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT
  

  // solution 1
  // if we can't get the lock
  if (atomic_exchange(&(ring->lock), 1) == 1){
    // exit
    return 1;
  }

  // if we can't pull
  if (!can_pull(ring)) {
    //exit
    atomic_store(&(ring->lock), 0);
    return 1;
  }

  *data = ring->elements[ring->tail % ring->size];
  ring->tail++;

  // let go of lock
  atomic_store(&(ring->lock), 0);

  return 0;
}
