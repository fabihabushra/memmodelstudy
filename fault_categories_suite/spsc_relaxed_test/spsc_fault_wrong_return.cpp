#include <relacy/relacy.hpp>
#include <cstddef>
#include <cstdint>

static constexpr std::size_t QUEUE_SIZE = 2;
static constexpr int NUM_VALUES = 2;
static constexpr int ROUNDS = 6;

#define LOAD_HEAD(q, mo)      ((q)->head.load((mo)))
#define STORE_HEAD(q, v, mo)  ((q)->head.store((v), (mo)))
#define LOAD_TAIL(q, mo)      ((q)->tail.load((mo)))
#define STORE_TAIL(q, v, mo)  ((q)->tail.store((v), (mo)))

#define LOAD_SLOT(q, idx)     ((q)->data[(idx)]($))
#define STORE_SLOT(q, idx, v) ((q)->data[(idx)]($) = (v))

#define SCHEDULE_POINT()      rl::yield(1, $)

struct SPSCQueue {
    rl::var<std::uint64_t> data[QUEUE_SIZE];
    rl::atomic<std::size_t> head;
    rl::atomic<std::size_t> tail;

    void init() {
        STORE_HEAD(this, 0, rl::memory_order_relaxed);
        STORE_TAIL(this, 0, rl::memory_order_relaxed);
        for (std::size_t i = 0; i < QUEUE_SIZE; ++i) {
            STORE_SLOT(this, i, 0);
        }
    }

    bool enqueue(std::uint64_t value) {
        std::size_t tail_val = LOAD_TAIL(this, rl::memory_order_relaxed);
        std::size_t head_val = LOAD_HEAD(this, rl::memory_order_acquire);
        std::size_t next_tail = (tail_val + 1) % QUEUE_SIZE;

        if (next_tail == head_val) {
            return false;
        }

        STORE_SLOT(this, tail_val, value);
        STORE_TAIL(this, next_tail, rl::memory_order_release);
        return true;
    }

    bool dequeue(std::uint64_t& value) {
        std::size_t head_val = LOAD_HEAD(this, rl::memory_order_relaxed);
        std::size_t tail_val = LOAD_TAIL(this, rl::memory_order_acquire);

        // faulty: claims success when empty
        if (head_val == tail_val) {
            value = LOAD_SLOT(this, head_val);
            return true;
        }

        value = LOAD_SLOT(this, head_val);
        STORE_HEAD(this, (head_val + 1) % QUEUE_SIZE, rl::memory_order_release);
        return true;
    }
};

struct spsc_test : rl::test_suite<spsc_test, 2> {
    SPSCQueue q;

    void before() {
        q.init();
    }

    void thread(unsigned tid) {
        if (tid == 0) {
            for (int r = 0; r < ROUNDS; ++r) {
                for (std::uint64_t v = 1; v <= NUM_VALUES; ++v) {
                    SCHEDULE_POINT();
                    while (!q.enqueue(v)) {
                        SCHEDULE_POINT();
                    }
                    SCHEDULE_POINT();
                }
            }
        } else {
            for (int r = 0; r < ROUNDS; ++r) {
                for (std::uint64_t expected = 1; expected <= NUM_VALUES; ++expected) {
                    std::uint64_t value = 0;
                    SCHEDULE_POINT();
                    while (!q.dequeue(value)) {
                        SCHEDULE_POINT();
                    }
                    RL_ASSERT(value == expected);
                    SCHEDULE_POINT();
                }
            }
        }
    }
};

int main() {
    rl::simulate<spsc_test>();
    return 0;
}
