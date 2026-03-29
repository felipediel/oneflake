# Advanced Usage

Once you move beyond the defaults, Oneflake stops being just a generator and becomes a tool for **designing how your IDs behave under scale**. At this stage, you're no longer simply generating IDs — you’re shaping the constraints of your system.

At its core, every ID encodes three components: time, worker identity, and sequence. Time represents the number of ticks since `epoch_ms`, the worker identifies which generator produced the ID, and the sequence counts how many IDs were created within the same tick. All of this must fit within a fixed 63-bit structure.

Because this space is fixed, every configuration choice becomes a trade-off. You are deciding how far into the future your IDs can extend, how many generators can run simultaneously, and how many IDs each generator can produce within a given time slice. These dimensions — time, parallelism, and throughput — all compete for the same limited resource. As a result, every gain in one dimension comes at the cost of another.

From here, the rest of the configuration is about how you allocate those 63 bits.

## Bit allocation

```
timestamp_bits + worker_bits + sequence_bits = 63
```

This is the core constraint of the system: every capability must fit within this fixed budget.

Let `T`, `W`, and `S` represent `timestamp_bits`, `worker_bits`, and `sequence_bits`. Then:

- max ticks → `2**T - 1`
- max workers → `2**W - 1`
- max sequence → `2**S - 1`

In practice, these define the upper bounds of your system: how long it can run, how many generators it can support, and how much throughput each generator can achieve per tick.

### Example

```python
FlakeGenerator(
    worker_id=1,
    timestamp_bits=41,
    worker_bits=10,
    sequence_bits=12,
)
```

- ~2.2T ticks
- 1024 workers
- 4096 IDs per tick per worker

## Time unit

Time in Oneflake advances in discrete ticks rather than continuous wall-clock time. This timeline is defined by two parameters: how many ticks exist, and how much real time each tick represents. The number of ticks is controlled by `timestamp_bits`, while the duration of each tick is defined by `time_unit_ns`.

Choosing a smaller `time_unit_ns` increases precision, allowing more granular ordering of events, but shortens the overall lifespan of the system. Larger values extend the lifespan, but reduce temporal precision.

### Ticks per second

The rate at which time progresses internally is determined by `time_unit_ns`:

```
1_000_000_000 / time_unit_ns
```

Smaller time units produce more ticks per second, increasing temporal resolution while consuming the available timeline faster.

### Sequence capacity

Each tick can emit a limited number of IDs, defined by `sequence_bits`:

```
sequence_capacity = 2**sequence_bits
```

Combined with ticks per second, this defines the maximum throughput:

```
sequence_capacity × ticks_per_second
```

### Lifespan

The total lifespan of a configuration depends on how many ticks exist and how long each tick represents:

```
ticks = 2**timestamp_bits
```

```
lifespan = ticks × time_per_tick
```

For example:

```python
FlakeGenerator(
    worker_id=1,
    timestamp_bits=41,
    worker_bits=10,
    sequence_bits=12,
    time_unit_ns=1_000_000,  # 1 ms
)
```

- total ticks → `2**41 ≈ 2.2 trillion`
- time per tick → `1 ms`

```
lifespan ≈ 2.2e12 × 1 ms ≈ 69 years
```

This is the effective duration the generator can produce IDs before exhausting its timeline.

## Timeline anchor

While `timestamp_bits` and `time_unit_ns` define the length and resolution of your timeline, `epoch_ms` defines where that timeline begins.

```python
FlakeGenerator(worker_id=1, epoch_ms=1_700_000_000_000)
```

- defines where tick `0` starts
- does not affect capacity

Use a fixed, shared value across your system (for example, a deployment or system start time).

## Layout

The layout defines how bits are ordered inside the 63-bit value, without changing their meaning or capacity.

```python
from oneflake import LAYOUT_TWS, LAYOUT_TSW

FlakeGenerator(worker_id=1, layout=LAYOUT_TSW)
```

- `TWS` → timestamp | worker | sequence
- `TSW` → timestamp | sequence | worker

This only affects encoding and decoding. Both sides must agree on the layout.

## Worker identity

Each generator must have a unique identifier:

```python
FlakeGenerator(worker_id=42)
```

Maximum value:

```
2**worker_bits - 1
```

Assign uniquely per machine, container, or process. Never reuse the same `worker_id` across concurrent generators.

## Custom clock

You can replace the system clock if needed:

```python
def fixed_clock_ns():
    return 1_700_000_000_000_000_000

FlakeGenerator(worker_id=1, epoch_ms=0, clock_func=fixed_clock_ns)
```

The function must return nanoseconds since the Unix epoch. This is mainly useful for testing and simulations.
