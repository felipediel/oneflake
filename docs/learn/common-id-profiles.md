# Common ID profiles

This page describes **well-known 64-bit flake-style schemes** as variations in bit allocation, time resolution, and layout. It’s a conceptual guide — you can implement any of these profiles using the parameters described in *[Advanced Usage](./advanced-usage.md)*.

## Snowflake (Twitter)

This is the most common baseline: a large timestamp, moderate worker space, and strong per-millisecond throughput.

| Aspect    | Typical choice                         |
| --------- | -------------------------------------- |
| Timestamp | 41 bits                                |
| Worker    | 10 bits (1024 nodes)                   |
| Sequence  | 12 bits (4096 IDs per tick per worker) |
| Tick      | 1 ms (`time_unit_ns=1_000_000`)        |
| Layout    | `LAYOUT_TWS`                           |
| Epoch     | Application-specific                   |

Oneflake’s **defaults** follow this profile. It offers a balanced trade-off: a long timeline, enough workers for most deployments, and high burst capacity per millisecond.

```python
from oneflake import FlakeGenerator

FlakeGenerator(worker_id=42)
```

## Sonyflake (Sony)

This profile shifts capacity toward **more machines** by increasing worker bits and using coarser time steps. As a result, per-tick throughput is lower.

| Aspect    | Typical choice                       |
| --------- | ------------------------------------ |
| Timestamp | 39 bits                              |
| Worker    | 16 bits (65,536 nodes)               |
| Sequence  | 8 bits (256 IDs per tick per worker) |
| Tick      | 10 ms (`time_unit_ns=10_000_000`)    |
| Layout    | `LAYOUT_TSW`                         |
| Epoch     | Application-specific                 |

**When it fits:** large distributed systems with many nodes and moderate ID generation per node.

**Trade-off:** fewer IDs per unit of time per worker compared to a 1 ms / 12-bit sequence design.

```python
from oneflake import FlakeDecoder, FlakeGenerator, LAYOUT_TWS

generator = FlakeGenerator(
    worker_id=12_345,
    layout=LAYOUT_TSW,
    timestamp_bits=39,
    worker_bits=16,
    sequence_bits=8,
    time_unit_ns=10_000_000,
    epoch_ms=0,
)

decoder = FlakeDecoder(
    layout=LAYOUT_TSW,
    timestamp_bits=39,
    worker_bits=16,
    sequence_bits=8,
)
```

## Shard-oriented (Instagram)

> ⚠️ **Not fully supported in Oneflake (yet)**
> Oneflake reserves one bit for the sign, leaving **63 bits for allocation**.
> Because of this, the exact 41 / 13 / 10 split (64 bits total) does not fit.
>
> **Suggested adaptation:** reduce the sequence by one bit (41 / 13 / 9).
> This preserves shard capacity and timeline, while slightly reducing per-shard throughput.

In this profile, the **worker field represents a logical shard** (such as a database partition or routing key), not just a physical machine.

| Aspect    | Typical choice                        |
| --------- | ------------------------------------- |
| Timestamp | 41 bits                               |
| Worker    | 13 bits (8192 shards)                 |
| Sequence  | 10 bits (1024 IDs per tick per shard) |
| Tick      | 1 ms                                  |
| Layout    | `LAYOUT_TSW`                          |
| Epoch     | Application-specific                  |

**When it fits:** systems where IDs are used for routing, partitioning, or embedding shard information.

**Trade-off:** lower per-shard throughput compared to a higher sequence allocation.

```python
from oneflake import FlakeDecoder, FlakeGenerator, LAYOUT_TWS

generator = FlakeGenerator(
    worker_id=4_096,
    layout=LAYOUT_TWS,
    timestamp_bits=41,
    worker_bits=13,
    sequence_bits=9,  # adjusted from 10 → 9
    time_unit_ns=1_000_000,
    epoch_ms=1_288_834_974_657,
)

decoder = FlakeDecoder(
    layout=LAYOUT_TWS,
    timestamp_bits=41,
    worker_bits=13,
    sequence_bits=9,
)
```

## Comparing profiles

| Profile            | Timestamp | Worker | Sequence | Tick  | Emphasis             |
| ------------------ | --------- | ------ | -------- | ----- | -------------------- |
| **Snowflake**      | 41        | 10     | 12       | 1 ms  | Balanced             |
| **Sonyflake**      | 39        | 16     | 8        | 10 ms | Many nodes           |
| **Shard-oriented** | 41        | 13     | 10       | 1 ms  | Logical partitioning |
