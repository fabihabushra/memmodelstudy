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
*/
static inline void mpmc_init(mpmc_queue_t* q) {
    STORE(q->enqueue_pos, 0, memory_order_relaxed);
    STORE(q->dequeue_pos, 0, memory_order_relaxed);

    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
        STORE_SLOT_VALUE(q, i, 0);
        STORE(q->slots[i].seq, i, memory_order_relaxed);
    }
}

/*
==================================================
enqueue()
==================================================
Matches the correct relaxed C++ logic.
*/
static inline int enqueue(mpmc_queue_t* q, uint64_t value) {
    size_t pos;

    while (1) {
        pos = LOAD(q->enqueue_pos, memory_order_acquire);
        size_t idx = pos % QUEUE_SIZE;
        size_t seq = LOAD(q->slots[idx].seq, memory_order_acquire);

        if (seq == pos) {
            size_t expected = pos;
            if (CAS(q->enqueue_pos, expected, pos + 1,
                            memory_order_release,
                            memory_order_relaxed)) {
                STORE_SLOT_VALUE(q, idx, value);
                STORE(q->slots[idx].seq, pos + 1, memory_order_release);
                return 1;
            }
        } else if (seq < pos) {
            return 0;
        }
    }
}

/*
==================================================
dequeue()
==================================================
Matches the correct relaxed C++ logic.
*/
static inline int dequeue(mpmc_queue_t* q, uint64_t* value) {
    size_t pos;

    while (1) {
        pos = LOAD(q->dequeue_pos, memory_order_acquire);
        size_t idx = pos % QUEUE_SIZE;
        size_t seq = LOAD(q->slots[idx].seq, memory_order_acquire);

        if (seq == pos + 1) {
            size_t expected = pos;
            if (CAS(q->dequeue_pos, expected, pos + 1,
                            memory_order_release,
                            memory_order_relaxed)) {
                *value = LOAD_SLOT_VALUE(q, idx);
                STORE(q->slots[idx].seq, pos + QUEUE_SIZE, memory_order_release);
                return 1;
            }
        } else if (seq < pos + 1) {
            return 0;
        }
    }
}
