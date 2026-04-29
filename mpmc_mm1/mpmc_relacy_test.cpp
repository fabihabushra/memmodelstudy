#include <relacy/relacy.hpp>
#include <cstdint>
#include <cstddef>

#define QUEUE_SIZE 200
#define PRODUCERS 10
#define CONSUMERS 10
#define THREADS (PRODUCERS + CONSUMERS)
#define ITEMS_PER_PRODUCER 20
#define TOTAL_ITEMS (PRODUCERS * ITEMS_PER_PRODUCER)

#define memory_order_seq_cst rl::memory_order_seq_cst

#define LOAD(var)              ((var).load(memory_order_seq_cst))
#define STORE(var, value)       ((var).store((value), memory_order_seq_cst))
#define CAS(var, expected, desired)  ((var).compare_exchange_weak((expected), (desired), memory_order_seq_cst, memory_order_seq_cst))

#define LOAD_SLOT_VALUE(q, idx)          ((q)->slots[(idx)].value($))
#define STORE_SLOT_VALUE(q, idx, v)      ((q)->slots[(idx)].value($) = (v))

#define MPMC_EXTERNAL_STRUCT

struct RLSlot {
    rl::var<uint64_t> value;
    rl::atomic<size_t> seq;
};

struct RLQueue {
    RLSlot slots[QUEUE_SIZE];
    rl::atomic<size_t> enqueue_pos;
    rl::atomic<size_t> dequeue_pos;
};

typedef RLSlot mpmc_slot_t;
typedef RLQueue mpmc_queue_t;

#include "mpmc_queue.h"

// Adapter
static inline int enqueue_rl(RLQueue* q, uint64_t v) {
    return enqueue(reinterpret_cast<mpmc_queue_t*>(q), v);
}

static inline int dequeue_rl(RLQueue* q, uint64_t* v) {
    return dequeue(reinterpret_cast<mpmc_queue_t*>(q), v);
}

// ===============================
// Test Harness
// ===============================

struct test : rl::test_suite<test, THREADS> {
    RLQueue q;
    rl::var<int> consumed_count[TOTAL_ITEMS + 1];
    rl::atomic<int> total_consumed;

    void before() {
        mpmc_init(reinterpret_cast<mpmc_queue_t*>(&q));

        for (int i = 0; i <= TOTAL_ITEMS; ++i) {
            consumed_count[i]($) = 0;
        }

        total_consumed.store(0, memory_order_seq_cst);
    }

    void thread(unsigned tid) {
        if (tid < PRODUCERS) {
            int producer_id = static_cast<int>(tid);

            for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
                uint64_t value = static_cast<uint64_t>(
                    producer_id * ITEMS_PER_PRODUCER + i + 1
                );

                rl::yield(1, $);
                while (!enqueue_rl(&q, value)) {
                    rl::yield(1, $);
                }
                rl::yield(1, $);
            }
        } else {
            while (true) {
                int current_total = total_consumed.load(memory_order_seq_cst);
                if (current_total >= TOTAL_ITEMS) {
                    break;
                }

                uint64_t value = 0;
                rl::yield(1, $);

                if (!dequeue_rl(&q, &value)) {
                    rl::yield(1, $);
                    continue;
                }

                RL_ASSERT(value >= 1 && value <= (uint64_t)TOTAL_ITEMS);

                int old_count = consumed_count[value]($);
                consumed_count[value]($) = old_count + 1;

                RL_ASSERT(consumed_count[value]($) == 1);

                total_consumed.fetch_add(1, memory_order_seq_cst);
                rl::yield(1, $);
            }
        }
    }

    void after() {
        RL_ASSERT(total_consumed.load(memory_order_seq_cst) == TOTAL_ITEMS);

        for (int v = 1; v <= TOTAL_ITEMS; ++v) {
            RL_ASSERT(consumed_count[v]($) == 1);
        }
    }
};

int main() {
    rl::simulate<test>();
    return 0;
}
