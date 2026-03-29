# FAQ

## Is Oneflake compatible with Snowflake IDs?

Yes.

Oneflake’s **default configuration matches the standard Snowflake profile**:

- layout: `TWS`
- timestamp: 41 bits
- worker: 10 bits
- sequence: 12 bits
- time unit: 1 ms

However, **full compatibility also requires matching the epoch**.

Since Snowflake systems typically use application-specific epochs, IDs from different systems are only interoperable if all parameters align.

## Is Oneflake compatible with Sonyflake IDs?

Yes.

You can reproduce Sonyflake-style IDs by configuring:

- layout: `TSW`
- timestamp: 39 bits
- worker: 16 bits
- sequence: 8 bits
- time unit: 10 ms

See [Common ID Profiles](../learn/common-id-profiles.md) for a complete example.

As with Snowflake, compatibility requires matching **all parameters**, including epoch.

## Is the decoded timestamp Unix time?

No.

`decode()` returns a **relative timestamp in ticks**, not wall-clock time.

To convert to Unix time:

```text
unix_time_ns = (timestamp + epoch) * time_unit_ns
```

Where:

- `timestamp` → decoded value
- `epoch` → `FlakeGenerator.epoch` (already normalized to ticks)
- `time_unit_ns` → your configured tick size

## Can I share one generator across threads?

Yes.

`FlakeGenerator` is **thread-safe** and uses a lock-free design based on atomic operations. It is safe under high contention.

## Can I share a generator across processes or machines?

No.

The generator does **not coordinate across processes or nodes**.

For distributed systems:

- assign a **unique `worker_id` per instance**
- never reuse the same `worker_id` concurrently

Global uniqueness depends on correct worker ID allocation.

## Can I decode IDs without knowing generator settings?

Not reliably.

Decoding requires the same:

- layout
- bit widths

A mismatch will produce incorrect values or raise `LayoutDecodeError`.

## What does the `epoch` property represent?

`FlakeGenerator.epoch` is the epoch expressed in **tick units**, not milliseconds.

It is derived from:

```text
epoch = epoch_ms / time_unit_ns
```

This is the internal reference point used during generation.

## What happens if the clock goes backward?

Short-lived drift is retried internally.

If the clock continues to move backward beyond a bounded threshold, generation fails with:

- `ClockRollbackError`

This protects against reusing previously issued timestamps.

## What happens when sequence capacity is exceeded?

If too many IDs are generated within the same timestamp:

- the sequence counter overflows
- the generator **blocks until the next timestamp is observed**

This is expected behavior and not an error.

## What happens when timestamp bits are exhausted?

Generation fails with:

- `TimeOverflowError`

This means the configured timeline has been fully consumed.

To resolve:

- increase `timestamp_bits`
- increase `time_unit_ns` (longer ticks → longer lifespan)
- or rotate to a new epoch/profile

## Does Oneflake guarantee ordering?

Within a single generator:

- IDs are **monotonic (non-decreasing)**

Across multiple generators:

- ordering depends on clock synchronization and `worker_id`
- no global ordering guarantee is enforced
