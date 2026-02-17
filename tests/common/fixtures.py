"""Fixtures for testing."""

from typing import Callable

import pytest

from .constants import (
    DEFAULT_EPOCH_MS,
    DEFAULT_LAYOUT,
    DEFAULT_SEQUENCE_BITS,
    DEFAULT_TIME_UNIT_NS,
    DEFAULT_WORKER_BITS,
    DEFAULT_WORKER_ID,
)
from .factories import (
    BurstClock,
    FlakeDecoder,
    FlakeEncoder,
    SequenceClock,
)


@pytest.fixture
def sequence_clock_factory() -> Callable[..., SequenceClock]:
    """Factory to create a SequenceClock."""

    def factory(*timestamps: int) -> SequenceClock:
        return SequenceClock(*timestamps)

    return factory


@pytest.fixture
def burst_clock_factory() -> Callable[..., BurstClock]:
    """Factory to create a BurstClock."""

    def factory(burst_size: int = 8) -> BurstClock:
        return BurstClock(burst_size)

    return factory


@pytest.fixture
def encoder_factory() -> Callable[..., FlakeEncoder]:
    """Factory to create a FlakeEncoder."""

    def factory(
        *,
        worker_id: int = DEFAULT_WORKER_ID,
        epoch_ms: int = DEFAULT_EPOCH_MS,
        layout: int = DEFAULT_LAYOUT,
        worker_bits: int = DEFAULT_WORKER_BITS,
        sequence_bits: int = DEFAULT_SEQUENCE_BITS,
        time_unit_ns: int = DEFAULT_TIME_UNIT_NS,
    ) -> FlakeEncoder:
        return FlakeEncoder(
            worker_id=worker_id,
            epoch_ms=epoch_ms,
            layout=layout,
            worker_bits=worker_bits,
            sequence_bits=sequence_bits,
            time_unit_ns=time_unit_ns,
        )

    return factory


@pytest.fixture
def decoder_factory() -> Callable[..., FlakeDecoder]:
    """Factory to create a FlakeDecoder."""

    def factory(
        *,
        layout: int = DEFAULT_LAYOUT,
        worker_bits: int = DEFAULT_WORKER_BITS,
        sequence_bits: int = DEFAULT_SEQUENCE_BITS,
    ) -> FlakeDecoder:
        return FlakeDecoder(
            layout=layout,
            worker_bits=worker_bits,
            sequence_bits=sequence_bits,
        )

    return factory
