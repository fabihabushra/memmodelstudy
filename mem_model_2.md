## Memory Model 2
In this memory model, memory ordering guarantees are based on how atomic operations synchronize between threads. When one thread performs a **release** operation on an atomic object and another thread performs an **acquire** operation that reads from that same object, a synchronization relationship is established. This ensures that all memory writes that happened-before the release operation in the first thread become visible to the second thread after it completes the acquire operation.

In the absence of explicit synchronization between threads, there are no guarantees about the relative ordering of memory operations as observed by different threads. As a result, different threads may see memory updates in different orders, even if those updates occurred in a specific order within another thread.

This model provides the following memory orders:

1. `memory_order_relaxed`
2. `memory_order_acquire`
3. `memory_order_release`
4. `memory_order_acq_rel`

This consistency model gurantees,
* For `memory_order_relaxed`, no operation orders memory.
* For `memory_order_release` and `memory_order_acq_rel`, a store operation performs a release operation on the affected memory location.
* For `memory_order_acquire` and `memory_order_acq_rel`, a load operation performs an acquire operation on the affected memory location.

To perform memory operations, this model provides the following functions:

* `STORE(var, value, mo)` — stores the given value in the memory location `var` using the specified memory order `mo`
* `LOAD(var, mo)` — loads and returns the value stored in the memory location `var` using the specified memory order `mo`
* `CAS(var, expected, desired, mo_success, mo_failure)` — atomically compares the value in `var` with `expected`. If they are equal, it updates `var` to `desired` and returns `true`, using the memory order `mo_success`. Otherwise, it leaves `var` unchanged, updates `expected` with the current value of `var`, and returns `false`, using the memory order `mo_failure`.

---

### Example 1

Initially, we have:

```
data = 0
flag = 0
```

**Thread 1:**

```
data = 42
STORE(flag, 1, memory_order_release)
```

**Thread 2:**

```
if (LOAD(flag, memory_order_acquire) == 1) {
    r = data
}
```

**Explanation:**
If Thread 2 reads `flag == 1`, then the acquire operation guarantees that it will also see all writes that happened-before the release in Thread 1. Therefore, `r` will correctly read `42`, not `0`.

---

### Example 2

Initially, we have:

```
counter = 0
```

**Thread 1:**

```
expected = 0
success = CAS(counter, expected, 1,
                           memory_order_acq_rel,
                           memory_order_acquire)
```

**Thread 2:**

```
expected = 0
success = CAS(counter, expected, 2,
                           memory_order_acq_rel,
                           memory_order_acquire)
```

**Explanation:**
Both threads attempt to update `counter` from `0` to a new value. Since `CAS` is atomic, only one thread can succeed.

* Suppose Thread 1 executes first. It sees `counter == 0`, so the comparison succeeds, and it updates `counter` to `1`. The operation returns `true`.
* When Thread 2 executes, it finds that `counter != 0`, so the comparison fails. The operation returns `false`, and `expected` is updated to the current value of `counter` (which is `1`).

The use of `memory_order_acq_rel` ensures that the successful update both:

* **releases** prior writes (making them visible to other threads), and
* **acquires** visibility of writes from other threads.

This guarantees that all threads observe a consistent update to `counter`, even under concurrent access.

### Comprehension Quiz

**1.** In the absence of explicit synchronization, all threads will see memory updates in the same order.
   - [ ] True
   - [ ] False

**2.** A release operation on an atomic object only affects that specific atomic object.
   - [ ] True
   - [ ] False

**3.** `memory_order_relaxed` provides no ordering guarantees for memory operations.
   - [ ] True
   - [ ] False

**4.** When Thread 2 performs an acquire operation that reads from an atomic variable written by Thread 1 with release, Thread 2 sees all writes that happened-before the release in Thread 1.
   - [ ] True
   - [ ] False

**5.** What is the relationship established when one thread performs a release operation and another performs an acquire operation on the same atomic object?
   - A. No relationship
   - B. A synchronization relationship
   - C. A blocking relationship
   - D. A mutual exclusion relationship

**6.** In the example, why does Thread 2 correctly read `data = 42` instead of `data = 0`?
   - A. Thread 1 runs before Thread 2
   - B. The acquire operation synchronizes with the release operation, ensuring visibility of prior writes
   - C. Both threads use the same memory location
   - D. The store operation blocks Thread 2

**7.** Which memory order guarantees BOTH acquire and release semantics?
   - A. `memory_order_relaxed`
   - B. `memory_order_acquire`
   - C. `memory_order_acq_rel`
   - D. `memory_order_release`
