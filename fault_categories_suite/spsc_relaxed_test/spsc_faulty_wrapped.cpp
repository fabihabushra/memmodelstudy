#include <relacy/relacy.hpp>
#include <cstddef>
#include <cstdint>

static constexpr std::size_t QUEUE_SIZE = 2;
static constexpr int NUM_VALUES = 2;

struct SPSCQueue {
    rl::var<std::uint64_t> data[QUEUE_SIZE];
    rl::atomic<std::size_t> head;
    rl::atomic<std::size_t> tail;

    void init() {
        head.store(0, rl::memory_order_relaxed);
        tail.store(0, rl::memory_order_relaxed);

        for (std::size_t i = 0; i < QUEUE_SIZE; ++i) {
            data[i]($) = 0;
        }
    }

    bool enqueue(std::uint64_t value) {
        std::size_t tail_val = tail.load(rl::memory_order_relaxed);
        std::size_t head_val = head.load(rl::memory_order_relaxed); // faulty
        std::size_t next_tail = (tail_val + 1) % QUEUE_SIZE;

        if (next_tail == head_val) {
            return false;
        }

        // Same weak fault as before:
        // publish index first, then write payload, all relaxed
        tail.store(next_tail, rl::memory_order_relaxed);
        data[tail_val]($) = value;

        return true;
    }

    bool dequeue(std::uint64_t& value) {
        std::size_t head_val = head.load(rl::memory_order_relaxed);
        std::size_t tail_val = tail.load(rl::memory_order_relaxed); // faulty

        if (head_val == tail_val) {
            return false;
        }

        value = data[head_val]($);
        head.store((head_val + 1) % QUEUE_SIZE, rl::memory_order_relaxed);

        return true;
    }
};

struct spsc_faulty_wrapped_test : rl::test_suite<spsc_faulty_wrapped_test, 2> {
    static constexpr int ROUNDS = 6;
    SPSCQueue q;

    void before() {
        q.init();
    }

    void thread(unsigned tid) {
        if (tid == 0) {
            for (int r = 0; r < ROUNDS; ++r) {
                for (std::uint64_t v = 1; v <= NUM_VALUES; ++v) {
                    rl::yield(1, $);

                    while (!q.enqueue(v)) {
                        rl::yield(1, $);
                    }

                    rl::yield(1, $);
                }
            }
        } else {
            for (int r = 0; r < ROUNDS; ++r) {
                for (std::uint64_t expected = 1; expected <= NUM_VALUES; ++expected) {
                    std::uint64_t value = 0;

                    rl::yield(1, $);

                    while (!q.dequeue(value)) {
                        rl::yield(1, $);
                    }

                    RL_ASSERT(value == expected);

                    rl::yield(1, $);
                }
            }
        }
    }
};

int main() {
    rl::simulate<spsc_faulty_wrapped_test>();
    return 0;
}
