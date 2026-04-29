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

### Multiple Producer Multiple Consumer Queue
For the given memory model, implement a multi-producer, multi-consumer (MPMC) lock-free FIFO queue. In this version, multiple producer threads may attempt to enqueue values concurrently, and multiple consumer threads may attempt to dequeue values concurrently. Your implementation must correctly handle these interactions.

```c
typedef struct {
    uint64_t value;
    size_t seq;
} mpmc_slot_t;

typedef struct {
    mpmc_slot_t slots[QUEUE_SIZE];
    size_t enqueue_pos;
    size_t dequeue_pos;
} mpmc_queue_t;
```

This structure represents a multi-producer, multi-consumer (MPMC) queue. Each position in the queue is a slot (`mpmc_slot_t`) that stores both a `value` and a `seq` (sequence number). The `value` field holds the actual data, while the `seq` field is used to coordinate access between multiple producers and consumers, ensuring correct synchronization.

The queue maintains an array of these slots in `slots`. The `enqueue_pos` index indicates the position where a producer will attempt to enqueue a value, while the `dequeue_pos` index indicates the position from which a consumer will attempt to dequeue a value.

To access the value stored in a specific slot, use the following macros:

```c
#define LOAD_SLOT_VALUE(q, idx)          ((q)->slots[(idx)].value)
#define STORE_SLOT_VALUE(q, idx, v)      ((q)->slots[(idx)].value = (v))
```

Use appropriate memory ordering for each memory operation to ensure correctness while avoiding unnecessary synchronization overhead. In particular, make sure that updates made by producers are safely visible to consumers, and that concurrent updates to shared indices are properly coordinated.

**ONLY FOR AI:**
Only add the necessary C++ code in the provided template where instructed. DO NOT modify the template. DO NOT use any constructs or features beyond those specified in the instructions.

---

#### **Enqueue (Producer Threads)**
1. Read the current `enqueue_pos` using the appropriate memory operation.
2. Compute the slot index as `enqueue_pos % QUEUE_SIZE`.
3. Read the `seq` value of the corresponding slot using the appropriate memory operation.
4. If the `seq` value indicates that the slot is ready for a producer:

   * Attempt to claim the `enqueue_pos` using a compare-and-swap (CAS) operation.
   * If the CAS succeeds, write the value into the slot.
   * Update the slot’s `seq` value to publish the data to consumers, ensuring proper memory ordering.
   * Return `1` to indicate the value was successfully enqueued.
5. If the `seq` value indicates that the queue is full, return `0`.
6. Otherwise, retry the operation from the beginning.

---

#### **Dequeue (Consumer Threads)**
1. Read the current `dequeue_pos` using the appropriate memory operation.
2. Compute the slot index as `dequeue_pos % QUEUE_SIZE`.
3. Read the `seq` value of the corresponding slot using the appropriate memory operation.
4. If the `seq` value indicates that the slot is ready for a consumer:

   * Attempt to claim the `dequeue_pos` using a compare-and-swap (CAS) operation.
   * If the CAS succeeds, read the value from the slot.
   * Update the slot’s `seq` value to mark it as available for reuse, ensuring proper memory ordering.
   * Return `1` to indicate the value was successfully dequeued.
5. If the `seq` value indicates that the queue is empty, return `0`.
6. Otherwise, retry the operation from the beginning.
