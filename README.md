# Oneflake

[![PyPI version](https://img.shields.io/pypi/v/oneflake.svg)](https://pypi.org/project/oneflake/)
[![Python](https://img.shields.io/pypi/pyversions/oneflake.svg)](https://pypi.org/project/oneflake/)
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](./LICENSE)

Oneflake is a high-performance **64-bit ID generator** for distributed systems. It generates globally unique, compact, time-sortable IDs with minimal overhead.

## Documentation

For a complete guide, advanced usage, and detailed API reference, see the [documentation](https://felipediel.github.io/oneflake/).

## Installation

Install Oneflake via PyPI:

```bash
pip install oneflake
```

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

## Oneflake ID Structure

A Oneflake ID is a **64-bit integer** whose bits encode time, origin, and sequence information.

| Field | Bits | Description |
|------|------|-------------|
| Reserved | 1 | Sign bit, always `0` |
| Timestamp | T | Time since `epoch_ms`, measured in units of `time_unit_ns` |
| Worker ID | W | Identifier of the node or process generating the ID |
| Sequence | S | Counter for IDs generated within the same time tick |

The field sizes are configurable via `timestamp_bits`, `worker_bits`, and `sequence_bits`, where:

```
1 + T + W + S = 64
```

The field order depends on the selected layout.

## Motivation

Modern distributed systems often need identifiers that are compact, unique, time-sortable, and inexpensive to generate at massive scale. Since Twitter’s Snowflake first introduced a 64-bit, timestamp-based approach for this purpose, many derivatives have appeared — including Sonyflake, Instagram Sharding IDs, and various custom Snowflake-inspired layouts — each balancing timestamp precision, worker capacity, and per-node throughput differently.

Oneflake builds on this evolution by offering a unified, flexible, and high-performance engine that lets you adopt or customize these layouts seamlessly through a clean Python API backed by a lock-free C core.

## Acknowledgements

Inspired by [Twitter’s Snowflake](https://github.com/twitter-archive/snowflake) and [Sonyflake](https://github.com/sony/sonyflake) ID strategies.

## License

This project is licensed under the [MIT License](LICENSE).

## Author

Oneflake was created in 2026 by [Felipe Martins Diel](https://github.com/felipediel).
