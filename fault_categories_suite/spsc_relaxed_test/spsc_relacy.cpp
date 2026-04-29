#include <relacy/relacy_std.hpp>

struct test : rl::test_suite<test, 2> {
    rl::atomic<int> x;

    void before() {
        x($) = 0;
    }

    void thread(unsigned id) {
        if (id == 0) {
            x.store(1, rl::memory_order_relaxed);
        } else {
            int v = x.load(rl::memory_order_relaxed);
            RL_ASSERT(v == 0 || v == 1);
        }
    }
};

int main() {
    rl::simulate<test>();
}
