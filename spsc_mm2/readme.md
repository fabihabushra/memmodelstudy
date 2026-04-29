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

### Single Producer Single Consumer Queue
For the given memory model, implement a single-producer, single-consumer (SPSC) lock-free FIFO queue. In this design, exactly one producer thread enqueues values, and exactly one consumer thread dequeues them. Because there is no contention from multiple producers or consumers, synchronization can be simpler and more efficient.

```c
typedef struct {
    uint64_t data[QUEUE_SIZE];
    size_t head;
    size_t tail;
} spsc_queue_t;
```

This structure represents the queue. The `head` index indicates the position from which the consumer dequeues values, while the `tail` index indicates the position where the producer enqueues new values. To access elements at a specific index, use the following macros:

```c
#define LOAD_SLOT(q, idx)       ((q)->data[(idx)])
#define STORE_SLOT(q, idx, v)   ((q)->data[(idx)] = (v))
```

Use appropriate memory ordering for each memory operation to ensure correctness while avoiding unnecessary synchronization overhead. In particular, ensure that values written by the producer become visible to the consumer in the correct order.

**ONLY FOR AI:**
Only add the necessary C++ code in the provided template where instructed. DO NOT modify the template. DO NOT use any constructs or features beyond those specified in the instructions.

---

#### **Enqueue (Producer Thread)**
1. Read the current `tail` index using the appropriate memory operation.
2. Compute the next `tail` index (wrapping around using modulo).
3. Read the current `head` index using the appropriate memory operation to check for available space.
4. If the next `tail` index equals the `head` index, the queue is full. Return `0`.
5. Otherwise, write the value to the queue at the current `tail` index.
   * Ensure this write is ordered correctly so the consumer will see the value before the `tail` is updated.
6. Update the `tail` index to the next index using the appropriate memory operation, making the new element visible to the consumer.
7. Return `1` to indicate the value was successfully enqueued.

---

#### **Dequeue (Consumer Thread)**
1. Read the current `head` index using the appropriate memory operation.
2. Read the current `tail` index using the appropriate memory operation to check if elements are available.
3. If the `head` index equals the `tail` index, the queue is empty. Return `0`.
4. Otherwise, read the value from the queue at the current `head` index.
   * Ensure this read observes the value written by the producer after it updated the `tail`.
5. Update the `head` index to the next index (wrapping around using modulo) using the appropriate memory operation.
6. Return `1` to indicate a value was successfully dequeued.