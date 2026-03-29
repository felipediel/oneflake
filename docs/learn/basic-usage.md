# Basic Usage

This guide covers the essentials: generating and decoding IDs using the default configuration.

By default, Oneflake uses a Snowflake-style layout (41-bit timestamp | 10-bit worker | 12-bit sequence). Both the generator and decoder share the same defaults, so you don’t need to configure anything unless you want a custom setup (see *[Advanced Usage](advanced-usage.md)* for that).

## Generating an ID

Create a generator with a unique `worker_id`, then call `generate()`:

```python
from oneflake import FlakeGenerator

generator = FlakeGenerator(worker_id=1)
unique_id = generator.generate()

print(unique_id)
```

**Important:**
The `worker_id` must be unique across all processes generating IDs at the same time using the same configuration. This ensures there are no collisions.

## Decoding an ID

To extract the components of an ID, use `FlakeDecoder`:

```python
from oneflake import FlakeDecoder

decoder = FlakeDecoder()
parts = decoder.decode(unique_id)

print(parts)
```

The decoder must match the generator’s configuration (layout, bit sizes, etc.).
If both use defaults, no arguments are required.
