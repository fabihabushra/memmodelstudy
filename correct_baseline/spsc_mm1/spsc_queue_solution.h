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
*/
static inline int enqueue(spsc_queue_t* q, uint64_t value) {
    size_t tail = LOAD(q->tail);
    size_t head = LOAD(q->head);
    size_t next_tail = (tail + 1) % QUEUE_SIZE;

    if (next_tail == head) {
        return 0;
    }

    STORE_SLOT(q, tail, value);
    STORE(q->tail, next_tail);
    return 1;
}

/*
==================================================
dequeue()
==================================================
*/
static inline int dequeue(spsc_queue_t* q, uint64_t* value) {
    size_t head = LOAD(q->head);
    size_t tail = LOAD(q->tail);

    if (head == tail) {
        return 0;
    }

    *value = LOAD_SLOT(q, head);
    STORE(q->head, (head + 1) % QUEUE_SIZE);
    return 1;
}
