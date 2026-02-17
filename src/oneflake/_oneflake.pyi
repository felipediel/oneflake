"""Oneflake ID generator and decoder."""

from __future__ import annotations

from typing import Callable, NamedTuple

LAYOUT_TWS: int
LAYOUT_TSW: int

MAX_EPOCH_MS: int
MIN_EPOCH_MS: int
UNIX_EPOCH_MS: int

DEFAULT_WORKER_ID: int
DEFAULT_LAYOUT: int
DEFAULT_TIMESTAMP_BITS: int
DEFAULT_WORKER_BITS: int
DEFAULT_SEQUENCE_BITS: int
DEFAULT_TIME_UNIT_NS: int
DEFAULT_EPOCH_MS: int

class FlakeParts(NamedTuple):
    """Flake parts.

    A named tuple containing the timestamp, worker ID, and sequence number.

    Attributes:
        timestamp: Timestamp.
        worker_id: Worker ID.
        sequence: Sequence number.
    """

    timestamp: int
    worker_id: int
    sequence: int

class FlakeGenerator:
    """Flake generator.

    Generates 64-bit unique IDs.

    Attributes:
        worker_id: Worker identifier.
        layout: Layout.
        timestamp_bits: Timestamp bits.
        worker_bits: Worker bits.
        sequence_bits: Sequence bits.
        time_unit_ns: Time unit in nanoseconds.
        epoch_ms: Epoch in time units.
    """

    def __init__(
        self,
        worker_id: int = DEFAULT_WORKER_ID,
        *,
        epoch_ms: int = DEFAULT_EPOCH_MS,
        timestamp_bits: int = DEFAULT_TIMESTAMP_BITS,
        worker_bits: int = DEFAULT_WORKER_BITS,
        sequence_bits: int = DEFAULT_SEQUENCE_BITS,
        time_unit_ns: int = DEFAULT_TIME_UNIT_NS,
        layout: int = DEFAULT_LAYOUT,
        clock_func: Callable[[], int] | None = None,
    ) -> None:
        """Initialize the FlakeGenerator instance."""
        ...

    @property
    def worker_id(self) -> int:
        """Worker identifier.

        The identifier for the machine that generated the ID.
        """
        ...

    @property
    def layout(self) -> int:
        """Layout.

        The order of the timestamp, worker identifier, and sequence number.

        Can be one of:
        - LAYOUT_TWS: timestamp, worker_id, sequence
        - LAYOUT_TSW: timestamp, sequence, worker_id
        """
        ...

    @property
    def timestamp_bits(self) -> int:
        """Timestamp bits.

        The number of bits used to store the timestamp.
        """
        ...

    @property
    def worker_bits(self) -> int:
        """Worker bits.

        The number of bits used to store the worker identifier.
        """
        ...

    @property
    def sequence_bits(self) -> int:
        """Sequence bits.

        The number of bits used to store the sequence number.
        """
        ...

    @property
    def time_unit_ns(self) -> int:
        """Time unit in nanoseconds.

        The number of nanoseconds that defines one time unit.
        """
        ...

    @property
    def epoch(self) -> int:
        """Epoch.

        The number of time units since the Unix epoch.
        """
        ...

    def generate(self) -> int:
        """Generate a 64-bit unique ID.

        Returns:
            int: A 64-bit unique ID.
        """
        ...

class FlakeDecoder:
    """Flake decoder.

    Decodes 64-bit unique IDs into their timestamp, worker ID, and sequence
    number.

    Attributes:
        layout: Layout.
        timestamp_bits: Timestamp bits.
        worker_bits: Worker bits.
        sequence_bits: Sequence bits.
    """

    def __init__(
        self,
        *,
        timestamp_bits: int = DEFAULT_TIMESTAMP_BITS,
        worker_bits: int = DEFAULT_WORKER_BITS,
        sequence_bits: int = DEFAULT_SEQUENCE_BITS,
        layout: int = DEFAULT_LAYOUT,
    ) -> None:
        """Initialize the FlakeDecoder instance."""
        ...

    @property
    def layout(self) -> int:
        """Layout."""
        ...

    @property
    def timestamp_bits(self) -> int:
        """Timestamp bits."""
        ...

    @property
    def worker_bits(self) -> int:
        """Worker bits."""
        ...

    @property
    def sequence_bits(self) -> int:
        """Sequence bits."""
        ...

    def decode(self, unique_id: int) -> FlakeParts:
        """Decode a 64-bit unique ID.

        Args:
            unique_id: A 64-bit unique ID.

        Returns:
            FlakeParts: A named tuple containing the timestamp, worker ID, and
                sequence number.
        """
        ...

class OneflakeException(Exception):
    """Base exception for oneflake errors."""

class ConfigurationError(OneflakeException):
    """Configuration-related exception."""

class ClockError(OneflakeException):
    """Clock-related exception."""

class DecodeError(OneflakeException):
    """Decode-related exception."""

class InvalidLayoutError(ConfigurationError):
    """Invalid layout configuration."""

class InvalidBitWidthError(ConfigurationError):
    """Invalid bit-width configuration."""

class InvalidWorkerError(ConfigurationError):
    """Invalid worker ID for configured bit-width."""

class InvalidTimeUnitError(ConfigurationError):
    """Invalid time unit configuration."""

class InvalidEpochError(ConfigurationError):
    """Invalid epoch configuration."""

class InvalidClockFunctionError(ConfigurationError):
    """Invalid custom clock function."""

class TimeBeforeEpochError(ClockError):
    """Current time is before configured epoch."""

class TimeReadError(ClockError):
    """Time read failed."""

class ClockRollbackError(ClockError):
    """Clock moved backwards while generating IDs."""

class TimeOverflowError(ClockError):
    """Timestamp exceeded representable range."""

class LayoutDecodeError(DecodeError):
    """Invalid internal layout while decoding."""
