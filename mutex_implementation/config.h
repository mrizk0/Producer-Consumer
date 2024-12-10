/*
 *
 *  Producer-Consumer Lab
 *
 *  Copyright (c) 2022 Peter A. Dinda, Branden Ghena
 *
 *  Warning: This file will be overwritten before grading.
 */

#pragma once

#define DEBUG_OUTPUT        1 // have DEBUG()s print something
#define OUTPUT_SHOWS_THREAD 1 // show thread number in debugging output

// Used to to fake interrupts using itimers, one per thread
#define INTERRUPT_TIMER  ITIMER_REAL    // use realtime timer
#define INTERRUPT_SIGNAL SIGALRM        // what itimer sends on expiration


// Helper function to find a thread's number for debugging
// The main thread is number 1, then worker threads are numbered from there
uint64_t find_my_thread();


#if DEBUG_OUTPUT
  #if OUTPUT_SHOWS_THREAD
    #define DEBUG(fmt, args...) fprintf(stderr, "(%d) %s(%d): debug: " fmt, (int)find_my_thread(), __FILE__, __LINE__, ##args)
  #else
    #define DEBUG(fmt, args...) fprintf(stderr, "%s(%d): debug: " fmt, __FILE__, __LINE__, ##args)
  #endif
#else
  #define DEBUG(fmt, args...) // nothing
#endif

#if OUTPUT_SHOWS_THREAD
  #define ERROR(fmt, args...) fprintf(stderr, "(%d) %s(%d): ERROR: " fmt, (int)find_my_thread(), __FILE__, __LINE__, ##args)
  #define INFO(fmt, args...)  fprintf(stderr, "(%d) %s(%d): " fmt, (int)find_my_thread(), __FILE__, __LINE__, ##args)
#else
  #define ERROR(fmt, args...) fprintf(stderr, "%s(%d): ERROR: " fmt, __FILE__, __LINE__, ##args)
  #define INFO(fmt, args...)  fprintf(stderr, "%s(%d): " fmt, __FILE__, __LINE__, ##args)
#endif

