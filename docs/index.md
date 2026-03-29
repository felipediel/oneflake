# Welcome to Oneflake

Oneflake is a **high-performance 64-bit ID generator** for distributed systems. It generates globally unique, compact, time-sortable IDs with minimal overhead.

Built on a C core with atomic operations, Oneflake is designed for production workloads where latency and reliability matter.

## Quick example

```python
from oneflake import FlakeGenerator

generator = FlakeGenerator(worker_id=7)
unique_id = generator.generate()
print(unique_id)  # e.g. 2027553873090080768
```

## Features

- **Globally unique identifiers:** collision-free across distributed workers and isolated nodes.
- **High-performance C core:** optimized for ultra-fast ID generation in concurrent environments.
- **Lock-free architecture:** atomic operations avoid mutex contention and scale efficiently across threads.
- **Flexible ID layout:** configure the size and ordering of timestamp, worker, and sequence fields.
- **Custom time configuration:** control epoch and timestamp precision to fit system requirements.
- **Built-in decoding:** easily extract timestamp, worker ID, and sequence components from generated IDs.

## Motivation

Modern distributed systems often need identifiers that are compact, unique, time-sortable, and inexpensive to generate at massive scale. Since Twitter's Snowflake first introduced a 64-bit, timestamp-based approach for this purpose, many derivatives have appeared — including Sonyflake, Instagram Sharding IDs, and various custom Snowflake-inspired layouts — each balancing timestamp precision, worker capacity, and per-node throughput differently.

Oneflake builds on this evolution by offering a unified, flexible, and high-performance engine that lets you adopt or customize these layouts seamlessly through a clean Python API backed by a lock-free C core.
