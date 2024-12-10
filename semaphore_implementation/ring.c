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

  sem_init(&(ring -> lock), 0, 1);
  sem_init(&(ring -> vacant), 0, size);
  sem_init(&(ring -> occupied), 0, 0);

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


  // check if there are vacant slots
  while (sem_wait(&(ring->vacant)) != 0);

  // get lock
  while (sem_wait(&(ring->lock)) != 0);

  DEBUG("starting push of %p to %p\n", data, ring);

  ring->elements[ring->head % ring->size] = data;
  ring->head++;

  DEBUG("push done\n");

  // release lock
  sem_post(&(ring->lock));
  // add 1 to occupied slots
  sem_post(&(ring->occupied));


  return 0;
}

// note that this is NOT synchronized and so prone to races!
int ring_try_push(ring_t* ring, void* data) {
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT

  // check to see if there are vacant slots
  if (sem_trywait(&(ring->vacant)) != 0){
    return 1;
  }
  // check if we can get lock
  if (sem_trywait(&(ring->lock)) != 0){
    // release vacant slot we would have filled
    sem_post(&(ring->vacant));
    return 1;
  }

  ring->elements[ring->head % ring->size] = data;
  ring->head++;

  // release lock
  sem_post(&(ring->lock));
  // add 1 to occupied slots
  sem_post(&(ring->occupied));

  return 0;
}


// Consumer Side

// note that this is NOT synchronized and so prone to races!
int ring_pull(ring_t* ring, void** data) {
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT

  // wait for an occupied slot
  while (sem_wait(&(ring -> occupied)) != 0); 
  // wait for lock
  while (sem_wait(&(ring -> lock)) != 0); 
  
  DEBUG("starting pull from %p\n", ring);

  *data = ring->elements[ring->tail % ring->size];
  ring->tail++;

  DEBUG("pulled %p\n", *data);

  // release the lock
  sem_post(&(ring->lock));
  // add 1 to vacant slots
  sem_post(&(ring->vacant));


  return 0;
}

// note that this is NOT synchronized and so prone to races!
int ring_try_pull(ring_t* ring, void** data) {
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT

  // check to see if there is an occupied slot
  if (sem_trywait(&(ring -> occupied)) != 0){
    return 1;
  }
  // check to see if we can get the lock
  if (sem_trywait(&(ring -> lock)) != 0){
    
    // release occupied slot we got at the start
    sem_post(&(ring->occupied));
    return 1;
  }

  *data = ring->elements[ring->tail % ring->size];
  ring->tail++;

  // relese lock
  sem_post(&(ring->lock));

  // add 1 to vacant slots
  sem_post(&(ring->vacant));

  return 0;
}

