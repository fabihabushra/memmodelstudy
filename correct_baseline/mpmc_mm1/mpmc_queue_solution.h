#pragma once
#include <stddef.h>
#include <stdint.h>

#define QUEUE_SIZE 200

#ifndef memory_order_seq_cst
#define memory_order_seq_cst 0
#endif

/*
==================================================
Required Access Macros
==================================================
This file is the correct C solution for MPMC SC.
The struct definitions are overridden in the Relacy wrapper.
*/

#ifndef STORE
#define STORE(var, value)    ((var) = (value))
#endif

#ifndef LOAD
#define LOAD(var)        ((var))
#endif

#ifndef CAS
#define CAS(var, expected, desired)   (((var) == (expected)) ? (((var) = (desired)), 1) : 0)
#endif

#ifndef LOAD_SLOT_VALUE
#define LOAD_SLOT_VALUE(q, idx)          ((q)->slots[(idx)].value)
#endif

#ifndef STORE_SLOT_VALUE
#define STORE_SLOT_VALUE(q, idx, v)      ((q)->slots[(idx)].value = (v))
#endif


#ifndef LOAD_SLOT_SEQ
#define LOAD_SLOT_SEQ(q, idx)        ((q)->slots[(idx)].seq)
#endif

#ifndef STORE_SLOT_SEQ
#define STORE_SLOT_SEQ(q, idx, v)    ((q)->slots[(idx)].seq = (v))
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
    STORE(q->enqueue_pos, 0);
    STORE(q->dequeue_pos, 0);

    for (size_t i = 0; i < QUEUE_SIZE; i++) {
        STORE_SLOT_VALUE(q, i, 0);
        STORE(q->slots[i].seq, i);
    }
}

/*
==================================================
enqueue()
==================================================
Matches the correct C++ SC logic.
*/
static inline int enqueue(mpmc_queue_t* q, uint64_t value) {
    size_t pos;

    while (1) {
        pos = LOAD(q->enqueue_pos);
        size_t idx = pos % QUEUE_SIZE;
        // size_t seq = LOAD(q->slots[idx].seq);
        size_t seq = LOAD_SLOT_SEQ(q, idx);


        if (seq == pos) {
            size_t expected = pos;
            if (CAS(q->enqueue_pos, expected, pos + 1)) {
                STORE_SLOT_VALUE(q, idx, value);
                // STORE(q->slots[idx].seq, pos + 1);
                STORE_SLOT_SEQ(q, idx, pos + 1);
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
Matches the correct C++ SC logic.
*/
static inline int dequeue(mpmc_queue_t* q, uint64_t* value) {
    size_t pos;

    while (1) {
        pos = LOAD(q->dequeue_pos);
        size_t idx = pos % QUEUE_SIZE;
        // size_t seq = LOAD(q->slots[idx].seq);
        size_t seq = LOAD_SLOT_SEQ(q, idx);


        if (seq == pos + 1) {
            size_t expected = pos;
            if (CAS(q->dequeue_pos, expected, pos + 1)) {
                *value = LOAD_SLOT_VALUE(q, idx);
                // STORE(q->slots[idx].seq, pos + QUEUE_SIZE);
                STORE_SLOT_SEQ(q, idx, pos + QUEUE_SIZE);
                return 1;
            }
        } else if (seq < pos + 1) {
            return 0;
        }
    }
}
