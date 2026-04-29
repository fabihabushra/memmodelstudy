## Memory Model 1
This memory model assumes a single global order of memory operations that is consistent across all threads. Each thread still follows its own program order, and all threads agree on the same overall sequence of operations.

If one thread performs two writes in a given order, other threads must observe those writes in the same order. Likewise, if a thread observes a later write from another thread, it must also observe that thread’s earlier writes.

To perform memory operations, this model provides the following functions:

* `STORE(var, value)` — stores the given value in the memory location `var`
* `LOAD(var)` — loads and returns the value stored in the memory location `var`
* `CAS(var, expected, desired)` — atomically compares the value in `var` with `expected`. If they are equal, it updates `var` to `desired` and returns `true`. Otherwise, it leaves `var` unchanged, updates `expected` with the current value of `var`, and returns `false`.

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
