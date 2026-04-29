#include <relacy/relacy.hpp>
#include <cstddef>
#include <cstdint>

static constexpr std::size_t QUEUE_SIZE = 4;
static constexpr int PRODUCERS = 2;
static constexpr int CONSUMERS = 2;
static constexpr int THREADS = PRODUCERS + CONSUMERS;
static constexpr int ITEMS_PER_PRODUCER = 4;
static constexpr int TOTAL_ITEMS = PRODUCERS * ITEMS_PER_PRODUCER;

#define memory_order_relaxed rl::memory_order_relaxed
#define memory_order_acquire rl::memory_order_acquire
#define memory_order_release rl::memory_order_release

#define LOAD_ENQ_POS(q, mo)      ((q)->enqueue_pos.load((mo)))
#define STORE_ENQ_POS(q, v, mo)  ((q)->enqueue_pos.store((v), (mo)))

#define LOAD_DEQ_POS(q, mo)      ((q)->dequeue_pos.load((mo)))
#define STORE_DEQ_POS(q, v, mo)  ((q)->dequeue_pos.store((v), (mo)))

#define CAS_ENQ_POS(q, expected, desired, success_mo, fail_mo) \
    ((q)->enqueue_pos.compare_exchange_weak((expected), (desired), (success_mo), (fail_mo)))

#define CAS_DEQ_POS(q, expected, desired, success_mo, fail_mo) \
    ((q)->dequeue_pos.compare_exchange_weak((expected), (desired), (success_mo), (fail_mo)))

#define LOAD_SLOT_VALUE(q, idx)         ((q)->slots[(idx)].value($))
#define STORE_SLOT_VALUE(q, idx, v)     ((q)->slots[(idx)].value($) = (v))

#define LOAD_SLOT_SEQ(q, idx, mo)       ((q)->slots[(idx)].seq.load((mo)))
#define STORE_SLOT_SEQ(q, idx, v, mo)   ((q)->slots[(idx)].seq.store((v), (mo)))

#define SCHEDULE_POINT() rl::yield(1, $)

struct Slot {
    rl::var<std::uint64_t> value;
    rl::atomic<std::size_t> seq;
};

struct MPMCQueue {
    Slot slots[QUEUE_SIZE];
    rl::atomic<std::size_t> enqueue_pos;
    rl::atomic<std::size_t> dequeue_pos;

    void init() {
        STORE_ENQ_POS(this, 0, memory_order_relaxed);
        STORE_DEQ_POS(this, 0, memory_order_relaxed);

        for (std::size_t i = 0; i < QUEUE_SIZE; ++i) {
            STORE_SLOT_VALUE(this, i, 0);
            STORE_SLOT_SEQ(this, i, i, memory_order_relaxed);
        }
    }

    bool enqueue(std::uint64_t value) {
        std::size_t pos;

        while (true) {
            pos = LOAD_ENQ_POS(this, memory_order_acquire);
            std::size_t idx = pos % QUEUE_SIZE;
            std::size_t seq = LOAD_SLOT_SEQ(this, idx, memory_order_acquire);

            if (seq == pos) {
                std::size_t expected = pos;
                if (CAS_ENQ_POS(this, expected, pos + 1,
                                memory_order_release,
                                memory_order_relaxed)) {
                    STORE_SLOT_VALUE(this, idx, value);
                    STORE_SLOT_SEQ(this, idx, pos + 1, memory_order_release);
                    return true;
                }
            } else if (seq < pos) {
                return false;
            }

            SCHEDULE_POINT();
        }
    }

    bool dequeue(std::uint64_t& value) {
        std::size_t pos;

        while (true) {
            pos = LOAD_DEQ_POS(this, memory_order_acquire);
            std::size_t idx = pos % QUEUE_SIZE;
            std::size_t seq = LOAD_SLOT_SEQ(this, idx, memory_order_acquire);

            if (seq == pos + 1) {
                std::size_t expected = pos;
                if (CAS_DEQ_POS(this, expected, pos + 1,
                                memory_order_release,
                                memory_order_relaxed)) {
                    value = LOAD_SLOT_VALUE(this, idx);

                    // faulty: wrong sequence reset for reuse
                    STORE_SLOT_SEQ(this, idx, pos + 1, memory_order_release);
                    return true;
                }
            } else if (seq < pos + 1) {
                return false;
            }

            SCHEDULE_POINT();
        }
    }
};

struct mpmc_relaxed_seq_fault_bad_wrap_test
    : rl::test_suite<mpmc_relaxed_seq_fault_bad_wrap_test, THREADS> {
    MPMCQueue q;
    rl::var<int> consumed_count[TOTAL_ITEMS + 1];
    rl::atomic<int> total_consumed;

    void before() {
        q.init();

        for (int i = 0; i <= TOTAL_ITEMS; ++i) {
            consumed_count[i]($) = 0;
        }

        total_consumed.store(0, memory_order_relaxed);
    }

    void thread(unsigned tid) {
        if (tid < PRODUCERS) {
            int producer_id = static_cast<int>(tid);

            for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
                std::uint64_t value =
                    static_cast<std::uint64_t>(producer_id * ITEMS_PER_PRODUCER + i + 1);

                SCHEDULE_POINT();
                while (!q.enqueue(value)) {
                    SCHEDULE_POINT();
                }
                SCHEDULE_POINT();
            }
        } else {
            while (true) {
                int current_total = total_consumed.load(memory_order_acquire);
                if (current_total >= TOTAL_ITEMS) {
                    break;
                }

                std::uint64_t value = 0;
                SCHEDULE_POINT();

                if (!q.dequeue(value)) {
                    SCHEDULE_POINT();
                    continue;
                }

                RL_ASSERT(value >= 1 && value <= static_cast<std::uint64_t>(TOTAL_ITEMS));

                int old_count = consumed_count[value]($);
                consumed_count[value]($) = old_count + 1;

                RL_ASSERT(consumed_count[value]($) == 1);

                total_consumed.fetch_add(1, memory_order_release);
                SCHEDULE_POINT();
            }
        }
    }

    void after() {
        RL_ASSERT(total_consumed.load(memory_order_acquire) == TOTAL_ITEMS);

        for (int v = 1; v <= TOTAL_ITEMS; ++v) {
            RL_ASSERT(consumed_count[v]($) == 1);
        }
    }
};

int main() {
    rl::simulate<mpmc_relaxed_seq_fault_bad_wrap_test>();
    return 0;
}
