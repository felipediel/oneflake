# Thread Safety Model

`FlakeGenerator` is safe for concurrent use from multiple threads.

It uses a **lock-free design** based on atomic operations to ensure correctness under contention.

## Core mechanism

- internal state is stored as a single atomic value
- each `generate()` performs an optimistic read followed by a compare-and-swap (CAS) update
- successful CAS **linearizes** one unique `(timestamp, sequence)` pair

The implementation uses relaxed memory ordering; correctness relies on atomic state transitions rather than cross-thread visibility of intermediate values.

## Uniqueness guarantees

Within a single generator instance:

- each successful CAS produces a **globally unique ID**
- no two threads can claim the same `(timestamp, sequence)` pair

Even when multiple threads target the same timestamp:

- only one thread can claim each sequence value
- losing CAS attempts retry with the updated state
- sequence overflow forces a transition to the next timestamp before reuse

## Rollback handling

If the observed timestamp falls behind the previously committed state:

- generation retries for a bounded number of attempts
- persistent rollback results in `ClockRollbackError`

This prevents reuse of previously issued timestamps and preserves monotonicity.

## Practical guidance

- sharing a single generator across threads is fully supported
- the generator does **not coordinate across processes or machines**
- distributed deployments must assign **unique `worker_id` values per instance**
- global collision guarantees depend on correct worker ID allocation
