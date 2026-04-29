#include <relacy/relacy.hpp>
#include <cstdint>
#include <cstddef>

#define QUEUE_SIZE 200

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

#include "spsc_queue.h"

static inline int enqueue_rl(RLQueue* q, uint64_t v) {
    return enqueue(reinterpret_cast<spsc_queue_t*>(q), v);
}

static inline int dequeue_rl(RLQueue* q, uint64_t* v) {
    return dequeue(reinterpret_cast<spsc_queue_t*>(q), v);
}

struct test : rl::test_suite<test, 2> {
    RLQueue q;

    void before() {
        spsc_init(reinterpret_cast<spsc_queue_t*>(&q));
    }

    void thread(unsigned tid) {
        if (tid == 0) {
            for (int i = 1; i <= 2; i++) {
                while (!enqueue_rl(&q, i)) {
                    rl::yield(1, $);
                }
            }
        } else {
            for (int i = 1; i <= 2; i++) {
                uint64_t v = 0;
                while (!dequeue_rl(&q, &v)) {
                    rl::yield(1, $);
                }
                RL_ASSERT(v == i);
            }
        }
    }
};

int main() {
    rl::simulate<test>();
    return 0;
}
