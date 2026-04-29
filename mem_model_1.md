## Memory Model 1
This memory model assumes a single global order of memory operations that is consistent across all threads. Each thread still follows its own program order, and all threads agree on the same overall sequence of operations.

If one thread performs two writes in a given order, other threads must observe those writes in the same order. Likewise, if a thread observes a later write from another thread, it must also observe that thread’s earlier writes.

To perform memory operations, this model provides the following functions:

* `STORE(var, value)` — stores the given value in the memory location `var`
* `LOAD(var)` — loads and returns the value stored in the memory location `var`
* `CAS(var, expected, desired)` — atomically compares the value in `var` with `expected`. If they are equal, it updates `var` to `desired` and returns `true`. Otherwise, it leaves `var` unchanged, updates `expected` with the current value of `var`, and returns `false`.

---

### Example

Initially, the memory locations have the following values:
X = 0
Y = 0

**Thread 1:**

```
STORE(X, 1)
STORE(Y, 1)
```

**Thread 2:**

```
r1 = LOAD(Y)   // reads 1
r2 = LOAD(X)   // can this read 0?
```

**Answer:** No. If Thread 2 observes `Y = 1`, it must also observe `X = 1`, because Thread 1 wrote to `X` before writing to `Y`, and all threads must observe writes in the same order.

---

### Comprehension Quiz

**1.** What does this memory model assume?
- A. Each thread has its own independent memory
- B. A single global order of memory operations
- C. Memory operations happen randomly
- D. Threads do not follow program order

**2.** If a thread performs two writes in a specific order, how must other threads observe them?
- A. In reverse order
- B. In any order
- C. In the same order
- D. Only one write is visible

**3.** If a thread observes a later write from another thread, what must also be true?
- A. It may ignore earlier writes
- B. It must also observe earlier writes
- C. It must restart execution
- D. It will see only the latest value

**4.** In the example, Thread 2 reads `Y = 1`. What should it read for `X`?
- A. 0
- B. 1
- C. Either 0 or 1
- D. Undefined

**5.** Why can Thread 2 not read `X = 0` after seeing `Y = 1`?
- A. Because X is never written
- B. Because reads happen before writes
- C. Because writes must be observed in order
- D. Because threads are independent
