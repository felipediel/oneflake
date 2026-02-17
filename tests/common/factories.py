"""Factories for testing."""

from threading import Lock
from typing import NamedTuple

from .constants import LAYOUT_TSW, LAYOUT_TWS
from .utils import to_ns


class FlakeParts(NamedTuple):
    """Flake parts."""

    timestamp: int
    worker_id: int
    sequence: int


class FlakeEncoder:
    """Encoder for flake IDs."""

    def __init__(
        self,
        *,
        worker_id: int,
        layout: int,
        worker_bits: int,
        sequence_bits: int,
        time_unit_ns: int,
        epoch_ms: int,
    ) -> None:
        """Initialize the encoder."""
        self.worker_id = worker_id
        self.layout = layout
        self.worker_bits = worker_bits
        self.sequence_bits = sequence_bits
        self.time_unit_ns = time_unit_ns
        self.epoch = to_ns(milliseconds=epoch_ms) // time_unit_ns

    def encode(
        self,
        timestamp_ns: int,
        sequence: int,
    ) -> int:
        """Encode a flake ID."""
        timestamp = timestamp_ns // self.time_unit_ns
        timestamp -= self.epoch

        if self.layout == LAYOUT_TWS:
            return (
                (timestamp << (self.worker_bits + self.sequence_bits))
                | (self.worker_id << self.sequence_bits)
                | sequence
            )

        if self.layout == LAYOUT_TSW:
            return (
                (timestamp << (self.sequence_bits + self.worker_bits))
                | (sequence << self.worker_bits)
                | self.worker_id
            )

        raise ValueError("Unsupported layout")


class FlakeDecoder:
    """Decoder for flake IDs."""

    def __init__(
        self,
        *,
        layout: int,
        worker_bits: int,
        sequence_bits: int,
    ) -> None:
        """Initialize the decoder."""
        self.layout = layout
        self.worker_bits = worker_bits
        self.sequence_bits = sequence_bits

    def decode(self, unique_id: int) -> FlakeParts:
        """Decode a flake ID."""
        if self.layout == LAYOUT_TWS:
            timestamp = unique_id >> (self.worker_bits + self.sequence_bits)
            worker_id = (unique_id >> self.sequence_bits) & (
                (1 << self.worker_bits) - 1
            )
            sequence = unique_id & ((1 << self.sequence_bits) - 1)
            return FlakeParts(timestamp, worker_id, sequence)

        if self.layout == LAYOUT_TSW:
            timestamp = unique_id >> (self.sequence_bits + self.worker_bits)
            sequence = (unique_id >> self.worker_bits) & (
                (1 << self.sequence_bits) - 1
            )
            worker_id = unique_id & ((1 << self.worker_bits) - 1)
            return FlakeParts(timestamp, worker_id, sequence)

        raise ValueError("Unsupported layout")


class SequenceClock:
    """Sequence clock.

    This class produces a sequence of timestamps.
    """

    def __init__(self, *timestamps_ns: int) -> None:
        """Initialize the sequential clock generator."""
        if not timestamps_ns:
            timestamps_ns = (0,)

        self._iter = iter(timestamps_ns)
        self._last = timestamps_ns[-1]

    def __call__(self) -> int:
        """Generate the next timestamp."""
        self._last = next(self._iter, self._last)
        return self._last


class BurstClock:
    """Burst clock.

    This class produces the same timestamp in small bursts, then advances to
    the next timestamp, in a thread-safe way.
    """

    def __init__(self, burst_size: int) -> None:
        """Initialize the burst clock."""
        self._burst_size = burst_size
        self._calls = 0
        self._lock = Lock()

    def __call__(self) -> int:
        """Return the next timestamp."""
        with self._lock:
            call_index = self._calls
            self._calls += 1

        timestamp_ms = call_index // self._burst_size
        return to_ns(milliseconds=timestamp_ms)
