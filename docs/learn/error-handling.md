# Error Handling

Oneflake provides a **structured exception hierarchy** that allows callers to clearly distinguish between:

- **Configuration-time failures** (invalid setup)
- **Runtime clock issues** (time consistency and monotonicity)
- **Decode misuse or incompatibility**

All exceptions inherit from:

```python
class OneflakeException(Exception): ...
```

## Exception Hierarchy

### Configuration Errors

Raised during `FlakeGenerator` or `FlakeDecoder` initialization.

- `ConfigurationError`
- `InvalidLayoutError`
- `InvalidBitWidthError`
- `InvalidWorkerError`
- `InvalidTimeUnitError`
- `InvalidEpochError`
- `InvalidClockFunctionError`

These indicate **programmer or deployment mistakes** and should not be retried.

### Clock Errors

Raised during ID generation (`FlakeGenerator.generate()`).

- `ClockError`
- `TimeReadError`
- `TimeBeforeEpochError`
- `ClockRollbackError`
- `TimeOverflowError`

These indicate **runtime time-related issues**.

Some may be transient (e.g. clock read failures), while others are **non-recoverable** and require intervention.

### Decode Errors

Raised during decoding (`FlakeDecoder.decode()`).

- `DecodeError`
- `LayoutDecodeError`

These indicate **invalid inputs or mismatched configuration**.

## Recommended Handling Strategy

### Fail fast on configuration errors

Treat all `ConfigurationError` subclasses as **non-recoverable**:

```python
from oneflake import ConfigurationError, FlakeGenerator

try:
    generator = FlakeGenerator(worker_id=worker_id)
except ConfigurationError as exc:
    raise RuntimeError(f"Invalid oneflake configuration: {exc}") from exc
```

Do not retry — fix configuration instead.

### Treat clock errors as operational incidents

Clock-related failures should trigger **observability + mitigation**, not silent retries:

```python
from oneflake import ClockError

def next_id(generator) -> int:
    try:
        return generator.generate()
    except ClockError:
        # Emit metrics, logs, alerts, or trigger circuit breaker
        raise
```

### Validate decode inputs at boundaries

```python
from oneflake import DecodeError

def safe_decode(decoder, value: int):
    try:
        return decoder.decode(value)
    except DecodeError:
        # Bad input or mismatched layout
        return None
```

## Runtime Semantics

### Exception chaining (`TimeReadError`)

`TimeReadError` is a **wrapper error** raised when timestamp retrieval fails.

It may wrap underlying causes such as:

- `TimeBeforeEpochError` (timestamp < configured epoch)
- Failures from a custom `clock_func`

Internally, this uses exception chaining (`__cause__`) to preserve the original failure.

```python
try:
    generator.generate()
except TimeReadError as exc:
    cause = exc.__cause__  # inspect root cause
```

### Clock rollback detection

`ClockRollbackError` is raised when:

- The system clock moves backwards
- And recovery attempts exceed an internal threshold

This indicates a **serious system issue**, such as:

- NTP adjustments
- VM clock drift
- Misconfigured time sync

➡️ Recommended action: **alert immediately and stop ID generation**

### Sequence overflow behavior

When multiple IDs are generated within the same timestamp:

- The sequence counter increments
- If it overflows, the generator **blocks until the next timestamp is observed**

This is expected and **not an error**.

### Timestamp limits

`TimeOverflowError` occurs when:

- The timestamp exceeds the configured bit capacity

This is a **hard limit** based on:

- `timestamp_bits`
- `epoch`
- `time_unit_ns`

➡️ This is **not recoverable at runtime** — requires reconfiguration.

## Quick Reference

| Exception              | Category | Retry?   | Action          |
| ---------------------- | -------- | -------- | --------------- |
| `ConfigurationError`   | Setup    | ❌ No    | Fix config      |
| `TimeReadError`        | Runtime  | ⚠️ Maybe | Inspect cause   |
| `TimeBeforeEpochError` | Runtime  | ❌ No    | Fix clock/epoch |
| `ClockRollbackError`   | Runtime  | ❌ No    | Alert + stop    |
| `TimeOverflowError`    | Runtime  | ❌ No    | Reconfigure     |
| `DecodeError`          | Input    | ❌ No    | Validate input  |
