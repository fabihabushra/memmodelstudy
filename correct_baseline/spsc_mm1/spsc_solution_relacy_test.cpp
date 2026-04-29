#include <relacy/relacy.hpp>
#include <cstdint>
#include <cstddef>


#define QUEUE_SIZE 200

#define memory_order_seq_cst rl::memory_order_seq_cst

#define LOAD(var)              ((var).load(memory_order_seq_cst))
#define STORE(var, value)       ((var).store((value), memory_order_seq_cst))
#define CAS(var, expected, desired)  ((var).compare_exchange_weak((expected), (desired), memory_order_seq_cst, memory_order_seq_cst))

#define LOAD_SLOT(q, idx)       ((q)->data[(idx)]($))
#define STORE_SLOT(q, idx, v)   ((q)->data[(idx)]($) = (v))


// Tell header not to define its own struct
#define SPSC_EXTERNAL_STRUCT

// Relacy struct
struct RLQueue {
    rl::var<uint64_t> data[QUEUE_SIZE];
    rl::atomic<size_t> head;
    rl::atomic<size_t> tail;
};

// IMPORTANT: alias C struct to Relacy struct
typedef RLQueue spsc_queue_t;

// Include C solution AFTER typedef
#include "spsc_queue_solution.h"

// Adapter
static inline int enqueue_rl(RLQueue* q, uint64_t v) {
    return enqueue((spsc_queue_t*)q, v);
}

static inline int dequeue_rl(RLQueue* q, uint64_t* v) {
    return dequeue((spsc_queue_t*)q, v);
}

// ===============================
// Test Harness
// ===============================

struct test : rl::test_suite<test, 2> {
    static constexpr int ROUNDS = 6;
    RLQueue q;

    void before() {
        spsc_init((spsc_queue_t*)&q);
    }

    void thread(unsigned tid) {
        if (tid == 0) {
            for (int r = 0; r < ROUNDS; ++r) {
                for (uint64_t i = 1; i <= 2; i++) {
                    rl::yield(1, $);
                    while (!enqueue_rl(&q, i)) {
                        rl::yield(1, $);
                    }
                    rl::yield(1, $);
                }
            }
        } else {
            for (int r = 0; r < ROUNDS; ++r) {
                for (uint64_t i = 1; i <= 2; i++) {
                    uint64_t v = 0;
                    rl::yield(1, $);
                    while (!dequeue_rl(&q, &v)) {
                        rl::yield(1, $);
                    }
                    RL_ASSERT(v == i);
                    rl::yield(1, $);
                }
            }
        }
    }
};

int main() {
    rl::simulate<test>();
    return 0;
}
