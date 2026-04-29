#include <relacy/relacy.hpp>
#include <cstdint>
#include <cstddef>

#define QUEUE_SIZE 200
#define NUM_VALUES 200

#define memory_order_relaxed rl::memory_order_relaxed
#define memory_order_acquire rl::memory_order_acquire
#define memory_order_release rl::memory_order_release
#define memory_order_acq_rel rl::memory_order_acq_rel

#define LOAD(var, mo)              ((var).load(mo))
#define STORE(var, value, mo)       ((var).store((value), mo))
#define CAS(var, expected, desired, success_mo, fail_mo)  ((var).compare_exchange_weak((expected), (desired), (success_mo), (fail_mo)))

#define LOAD_SLOT(q, idx)       ((q)->data[(idx)]($))
#define STORE_SLOT(q, idx, v)   ((q)->data[(idx)]($) = (v))

#define SPSC_EXTERNAL_STRUCT

struct RLQueue {
    rl::var<uint64_t> data[QUEUE_SIZE];
    rl::atomic<size_t> head;
    rl::atomic<size_t> tail;
};

typedef RLQueue spsc_queue_t;

#include "spsc_queue_solution.h"

static inline int enqueue_rl(RLQueue* q, uint64_t v) {
    return enqueue(reinterpret_cast<spsc_queue_t*>(q), v);
}

static inline int dequeue_rl(RLQueue* q, uint64_t* v) {
    return dequeue(reinterpret_cast<spsc_queue_t*>(q), v);
}

struct test : rl::test_suite<test, 2> {
    RLQueue q;
    uint64_t consumed[NUM_VALUES];

    void before() {
        spsc_init(reinterpret_cast<spsc_queue_t*>(&q));
        for (int i = 0; i < NUM_VALUES; ++i) {
            consumed[i] = 9999;
        }
    }

    void thread(unsigned tid) {
        if (tid == 0) {
            for (uint64_t v = 1; v <= NUM_VALUES; ++v) {
                while (!enqueue_rl(&q, v)) {
                    rl::yield(1, $);
                }
            }
        } else {
            for (int i = 0; i < NUM_VALUES; ++i) {
                uint64_t value = 0;
                while (!dequeue_rl(&q, &value)) {
                    rl::yield(1, $);
                }
                consumed[i] = value;
            }
        }
    }

    void after() {
        RL_ASSERT(consumed[0] == 1);
        RL_ASSERT(consumed[1] == 2);
    }
};

int main() {
    rl::simulate<test>();
    return 0;
}
