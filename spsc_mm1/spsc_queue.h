#pragma once
#include <stddef.h>
#include <stdint.h>

#define QUEUE_SIZE 200

/*
==================================================
Required Access Macros
==================================================
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

#ifndef STORE_SLOT
#define STORE_SLOT(q, idx, v)   ((q)->data[(idx)] = (v))
#endif

#ifndef LOAD_SLOT
#define LOAD_SLOT(q, idx)       ((q)->data[(idx)])
#endif

/*
==================================================
Struct Definition
==================================================
This is overridden in Relacy wrapper.
*/

#ifndef SPSC_EXTERNAL_STRUCT
typedef struct {
    uint64_t data[QUEUE_SIZE];
    size_t head;
    size_t tail;
} spsc_queue_t;
#endif

/*
==================================================
Initialization
==================================================
Do not modify this function.
*/
static inline void spsc_init(spsc_queue_t* q) {
    STORE(q->head, 0);
    STORE(q->tail, 0);

    for (size_t i = 0; i < QUEUE_SIZE; i++) {
        STORE_SLOT(q, i, 0);
    }
}

/*
==================================================
enqueue()
==================================================

Suggested steps:
1. Load tail
2. Load head
3. Compute next_tail
4. If queue is full, return 0
5. Store value into the current tail slot
6. Publish next_tail
7. Return 1
*/
static inline int enqueue(spsc_queue_t* q, uint64_t value) {
    // TODO: implement

    // Example of required macro style:
    // size_t tail = LOAD(q->tail);

    return 0;
}

/*
==================================================
dequeue()
==================================================

Suggested steps:
1. Load head
2. Load tail
3. If queue is empty, return 0
4. Read the value from the current head slot
5. Publish updated head
6. Return 1
*/
static inline int dequeue(spsc_queue_t* q, uint64_t* value) {
    // TODO: implement

    // Example of required macro style:
    // size_t head = LOAD(q->head);

    return 0;
}
