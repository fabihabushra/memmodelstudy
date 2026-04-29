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
The struct is overridden by the Relacy wrapper.
*/
#ifndef LOAD
#define LOAD(var, mo)        ((var))
#endif

#ifndef STORE
#define STORE(var, value, mo)    ((var) = (value))
#endif

#ifndef CAS
#define CAS(var, expected, desired, success_mo, fail_mo)   (((var) == (expected)) ? (((var) = (desired)), 1) : 0)
#endif

#ifndef LOAD_SLOT
#define LOAD_SLOT(q, idx)       ((q)->data[(idx)])
#endif

#ifndef STORE_SLOT
#define STORE_SLOT(q, idx, v)   ((q)->data[(idx)] = (v))
#endif

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
    STORE(q->head, 0, memory_order_relaxed);
    STORE(q->tail, 0, memory_order_relaxed);

    for (size_t i = 0; i < QUEUE_SIZE; i++) {
        STORE_SLOT(q, i, 0);
    }
}

/*
==================================================
enqueue()
==================================================
- tail load: relaxed
- head load: acquire
- tail store: release
*/
static inline int enqueue(spsc_queue_t* q, uint64_t value) {
    size_t tail = LOAD(q->tail, memory_order_relaxed);
    size_t head = LOAD(q->head, memory_order_acquire);
    size_t next_tail = (tail + 1) % QUEUE_SIZE;

    if (next_tail == head) {
        return 0;
    }

    STORE_SLOT(q, tail, value);
    STORE(q->tail, next_tail, memory_order_release);
    return 1;
}

/*
==================================================
dequeue()
==================================================
- head load: relaxed
- tail load: acquire
- head store: release
*/
static inline int dequeue(spsc_queue_t* q, uint64_t* value) {
    size_t head = LOAD(q->head, memory_order_relaxed);
    size_t tail = LOAD(q->tail, memory_order_acquire);

    if (head == tail) {
        return 0;
    }

    *value = LOAD_SLOT(q, head);
    STORE(q->head, (head + 1) % QUEUE_SIZE, memory_order_release);
    return 1;
}
