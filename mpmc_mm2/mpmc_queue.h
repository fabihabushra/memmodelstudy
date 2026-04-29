#pragma once
#include <stddef.h>
#include <stdint.h>

#define QUEUE_SIZE 200

#ifndef memory_order_relaxed
#define memory_order_relaxed 0
#endif

#ifndef memory_order_acquire
#define memory_order_acquire 1
#endif

#ifndef memory_order_release
#define memory_order_release 2
#endif

#ifndef memory_order_acq_rel
#define memory_order_acq_rel 3
#endif

/*
==================================================
Required Access Macros
==================================================
*/

#ifndef STORE
#define STORE(var, value, mo)    ((var) = (value))
#endif

#ifndef LOAD
#define LOAD(var, mo)        ((var))
#endif

#ifndef CAS
#define CAS(var, expected, desired, success_mo, fail_mo)   (((var) == (expected)) ? (((var) = (desired)), 1) : 0)
#endif

#ifndef LOAD_SLOT_VALUE
#define LOAD_SLOT_VALUE(q, idx)          ((q)->slots[(idx)].value)
#endif

#ifndef STORE_SLOT_VALUE
#define STORE_SLOT_VALUE(q, idx, v)      ((q)->slots[(idx)].value = (v))
#endif

/*
==================================================
Struct Definitions
==================================================
These are overridden in the Relacy wrapper.
*/
#ifndef MPMC_EXTERNAL_STRUCT
typedef struct {
    uint64_t value;
    size_t seq;
} mpmc_slot_t;

typedef struct {
    mpmc_slot_t slots[QUEUE_SIZE];
    size_t enqueue_pos;
    size_t dequeue_pos;
} mpmc_queue_t;
#endif

/*
==================================================
Initialization
==================================================
Do not modify this function.
*/
static inline void mpmc_init(mpmc_queue_t* q) {
    STORE(q->enqueue_pos, 0, memory_order_relaxed);
    STORE(q->dequeue_pos, 0, memory_order_relaxed);

    for (size_t i = 0; i < QUEUE_SIZE; i++) {
        STORE_SLOT_VALUE(q, i, 0);
        STORE(q->slots[i].seq, i, memory_order_relaxed);
    }
}

/*
==================================================
enqueue()
==================================================

Suggested steps:
1. Load enqueue position
2. Compute slot index = pos % QUEUE_SIZE
3. Load slot sequence
4. If slot sequence indicates slot is ready for producer:
      - claim enqueue position using CAS
      - write payload
      - publish slot sequence
      - return 1
5. If slot sequence indicates queue is full, return 0
6. Otherwise retry
*/
static inline int enqueue(mpmc_queue_t* q, uint64_t value) {
    // TODO: implement

    // Example of required macro style:
    // size_t pos = LOAD(q->enqueue_pos, memory_order_acquire);

    return 0;
}

/*
==================================================
dequeue()
==================================================

Suggested steps:
1. Load dequeue position
2. Compute slot index = pos % QUEUE_SIZE
3. Load slot sequence
4. If slot sequence indicates slot is ready for consumer:
      - claim dequeue position using CAS
      - read payload
      - publish recycled slot sequence
      - return 1
5. If slot sequence indicates queue is empty, return 0
6. Otherwise retry
*/
static inline int dequeue(mpmc_queue_t* q, uint64_t* value) {
    // TODO: implement

    // Example of required macro style:
    // size_t pos = LOAD(q->dequeue_pos, memory_order_acquire);

    return 0;
}
