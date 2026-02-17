"""Tests for FlakeGenerator."""

from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor
from threading import Barrier
from typing import Callable

import pytest

from oneflake import (
    ClockRollbackError,
    FlakeGenerator,
    InvalidBitWidthError,
    InvalidClockFunctionError,
    InvalidEpochError,
    InvalidLayoutError,
    InvalidTimeUnitError,
    InvalidWorkerError,
    TimeBeforeEpochError,
    TimeOverflowError,
    TimeReadError,
)

from tests.common.constants import (
    DEFAULT_EPOCH_MS,
    LAYOUT_TSW,
    LAYOUT_TWS,
    MAX_EPOCH_MS,
    TIME_UNITS_NS,
    UNIX_EPOCH_MS,
)
from tests.common.factories import (
    BurstClock,
    FlakeDecoder,
    SequenceClock,
)
from tests.common.utils import to_ns


class TestFlakeGeneratorInit:
    """Tests for the FlakeGenerator.__init__ method."""

    def test_init_sets_defaults_when_no_arguments_are_provided(self) -> None:
        """
        Test that the initialization method sets the defaults when no arguments
        are provided.
        """
        flake_generator = FlakeGenerator()

        assert flake_generator.worker_id == 0
        assert flake_generator.layout == LAYOUT_TWS
        assert flake_generator.timestamp_bits == 41
        assert flake_generator.worker_bits == 10
        assert flake_generator.sequence_bits == 12
        assert flake_generator.time_unit_ns == 1_000_000
        assert flake_generator.epoch == 1_288_834_974_657

    def test_init_sets_custom_values_when_arguments_are_provided(self) -> None:
        """
        Test that the initialization method sets custom values when arguments
        are provided.
        """
        flake_generator = FlakeGenerator(
            worker_id=7,
            layout=LAYOUT_TSW,
            timestamp_bits=50,
            worker_bits=8,
            sequence_bits=5,
            time_unit_ns=1_000,
            epoch_ms=123,
        )

        assert flake_generator.worker_id == 7
        assert flake_generator.layout == LAYOUT_TSW
        assert flake_generator.timestamp_bits == 50
        assert flake_generator.worker_bits == 8
        assert flake_generator.sequence_bits == 5
        assert flake_generator.time_unit_ns == 1_000
        assert flake_generator.epoch == 123_000

    def test_init_raises_type_error_when_keyword_only_args_are_passed_positionally(
        self,
    ) -> None:
        """
        Test that the initialization method raises a TypeError when
        keyword-only arguments are passed positionally.
        """
        with pytest.raises(TypeError):
            FlakeGenerator(1, LAYOUT_TSW)  # ty: ignore[too-many-positional-arguments]

    @pytest.mark.parametrize("layout", [-1, 2])
    def test_init_raises_invalid_layout_error_when_layout_is_out_of_range(
        self, layout: int
    ) -> None:
        """
        Test that the initialization method raises an InvalidLayoutError when
        the layout argument is out of range.
        """
        with pytest.raises(
            InvalidLayoutError,
            match="Layout must be 0 \\(TWS\\) or 1 \\(TSW\\)",
        ):
            FlakeGenerator(layout=layout)

    def test_init_raises_overflow_error_when_layout_is_too_large_to_convert(
        self,
    ) -> None:
        """
        Test that the initialization method raises an OverflowError when the
        layout argument is too large to convert to an integer.
        """
        with pytest.raises(OverflowError):
            FlakeGenerator(layout=2**128)

    @pytest.mark.parametrize("timestamp_bits", [-1, 0, 62])
    def test_init_raises_invalid_bit_width_error_when_timestamp_bits_is_out_of_range(
        self, timestamp_bits: int
    ) -> None:
        """
        Test that the initialization method raises an InvalidBitWidthError when
        the timestamp bits argument is out of range.
        """
        with pytest.raises(
            InvalidBitWidthError,
            match="Timestamp bits must be between 1 and 61",
        ):
            FlakeGenerator(timestamp_bits=timestamp_bits)

    @pytest.mark.parametrize("worker_bits", [-1, 0, 62])
    def test_init_raises_invalid_bit_width_error_when_worker_bits_is_out_of_range(
        self, worker_bits: int
    ) -> None:
        """
        Test that the initialization method raises an InvalidBitWidthError when
        the worker bits argument is out of range.
        """
        with pytest.raises(
            InvalidBitWidthError,
            match="Worker bits must be between 1 and 61",
        ):
            FlakeGenerator(worker_bits=worker_bits)

    @pytest.mark.parametrize("sequence_bits", [-1, 0, 62])
    def test_init_raises_invalid_bit_width_error_when_sequence_bits_is_out_of_range(
        self, sequence_bits: int
    ) -> None:
        """
        Test that the initialization method raises an InvalidBitWidthError when
        the sequence bits argument is out of range.
        """
        with pytest.raises(
            InvalidBitWidthError,
            match="Sequence bits must be between 1 and 61",
        ):
            FlakeGenerator(sequence_bits=sequence_bits)

    @pytest.mark.parametrize(
        "timestamp_bits, worker_bits, sequence_bits",
        [
            (39, 10, 13),
            (41, 10, 13),
            (40, 9, 13),
            (40, 11, 13),
            (40, 10, 12),
            (40, 10, 14),
        ],
    )
    def test_init_raises_invalid_bit_width_error_when_invalid_bit_configuration_is_provided(
        self,
        timestamp_bits: int,
        worker_bits: int,
        sequence_bits: int,
    ) -> None:
        """
        Test that the initialization method raises an InvalidBitWidthError when
        the sum of the timestamp bits, worker bits and sequence bits is not 63.
        """
        with pytest.raises(
            InvalidBitWidthError,
            match="The sum of timestamp bits, worker bits, and sequence bits must be 63",
        ):
            FlakeGenerator(
                timestamp_bits=timestamp_bits,
                worker_bits=worker_bits,
                sequence_bits=sequence_bits,
            )

    @pytest.mark.parametrize("time_unit_ns", [-1, 0, 2**41])
    def test_init_raises_invalid_time_unit_error_when_time_unit_is_out_of_range(
        self, time_unit_ns: int
    ) -> None:
        """
        Test that the initialization method raises an InvalidTimeUnitError when
        the time unit argument is out of range.
        """
        with pytest.raises(
            InvalidTimeUnitError,
            match=f"Time unit must be between 1 and {2**41 - 1}",
        ):
            FlakeGenerator(timestamp_bits=41, time_unit_ns=time_unit_ns)

    @pytest.mark.parametrize("worker_id", [-1, 4])
    def test_init_raises_invalid_worker_error_when_worker_id_is_out_of_range(
        self, worker_id: int
    ) -> None:
        """
        Test that the initialization method raises an InvalidWorkerError when
        the worker ID argument is out of range.
        """
        with pytest.raises(
            InvalidWorkerError, match="Worker ID must be between 0 and 3"
        ):
            FlakeGenerator(
                worker_id=worker_id,
                worker_bits=2,
                timestamp_bits=59,
                sequence_bits=2,
            )

    def test_init_raises_overflow_error_when_worker_id_is_too_large_to_convert(
        self,
    ) -> None:
        """
        Test that the initialization method raises an OverflowError when the
        worker_id argument is too large to convert to an integer.
        """
        with pytest.raises(OverflowError):
            FlakeGenerator(worker_id=2**128)

    @pytest.mark.parametrize("epoch_ms", [-1, MAX_EPOCH_MS + 1])
    def test_init_raises_invalid_epoch_error_when_epoch_is_out_of_range(
        self, epoch_ms: int
    ) -> None:
        """
        Test that the initialization method raises an InvalidEpochError when
        the epoch_ms argument is out of range.
        """
        with pytest.raises(
            InvalidEpochError,
            match=f"Epoch must be between 0 and {MAX_EPOCH_MS}",
        ):
            FlakeGenerator(epoch_ms=epoch_ms)

    def test_init_raises_overflow_error_when_epoch_is_too_large_to_convert(
        self,
    ) -> None:
        """
        Test that the initialization method raises an OverflowError when the
        epoch_ms argument is too large to convert to an integer.
        """
        with pytest.raises(OverflowError):
            FlakeGenerator(epoch_ms=2**128)

    def test_init_raises_invalid_clock_function_error_when_clock_func_is_not_callable(
        self,
    ) -> None:
        """
        Test that the initialization method raises an InvalidClockFunctionError
        when the clock_func argument is not a callable object.
        """
        with pytest.raises(
            InvalidClockFunctionError,
            match="The clock_func argument must be a callable object or None",
        ):
            FlakeGenerator(clock_func=123)  # ty: ignore[invalid-argument-type]

    def test_init_accepts_none_clock_func(self) -> None:
        """
        Test that the initialization method accepts None for the clock_func
        parameter.
        """
        try:
            FlakeGenerator(clock_func=None)
        except Exception as e:
            raise AssertionError(
                f"Initialization failed with None clock_func: {e}"
            ) from e


class TestFlakeGeneratorGenerate:
    """Tests for the FlakeGenerator.generate method."""

    def test_generate_returns_integer_result(self) -> None:
        """
        Test that the generate method returns an integer result.
        """
        flake_generator = FlakeGenerator(epoch_ms=0)
        unique_id = flake_generator.generate()
        assert isinstance(unique_id, int)

    @pytest.mark.parametrize("layout", [LAYOUT_TWS, LAYOUT_TSW])
    @pytest.mark.parametrize("time_unit_ns", TIME_UNITS_NS)
    @pytest.mark.parametrize("epoch_ms", [UNIX_EPOCH_MS, DEFAULT_EPOCH_MS])
    @pytest.mark.parametrize("relative_timestamp_ms", [0, 1, 2])
    def test_generate_returns_correct_timestamp(
        self,
        layout: int,
        time_unit_ns: int,
        epoch_ms: int,
        relative_timestamp_ms: int,
        sequence_clock_factory: Callable[..., SequenceClock],
        decoder_factory: Callable[..., FlakeDecoder],
    ) -> None:
        """
        Test that the generate method returns an identifier with the correct
        timestamp.
        """
        epoch_ns = to_ns(milliseconds=epoch_ms)
        relative_timestamp_ns = to_ns(milliseconds=relative_timestamp_ms)
        timestamp_ns = epoch_ns + relative_timestamp_ns
        clock_func = sequence_clock_factory(timestamp_ns)
        flake_generator = FlakeGenerator(
            layout=layout,
            time_unit_ns=time_unit_ns,
            epoch_ms=epoch_ms,
            clock_func=clock_func,
        )

        unique_id = flake_generator.generate()

        expected_timestamp = (
            timestamp_ns // time_unit_ns - epoch_ns // time_unit_ns
        )
        flake_decoder = decoder_factory(layout=layout)
        decoded_id = flake_decoder.decode(unique_id)
        assert decoded_id.timestamp == expected_timestamp

    @pytest.mark.parametrize("layout", [LAYOUT_TWS, LAYOUT_TSW])
    def test_generate_returns_correct_worker_id(
        self, layout: int, decoder_factory: Callable[..., FlakeDecoder]
    ) -> None:
        """
        Test that the generate method returns an identifier with the correct
        worker ID.
        """
        flake_generator = FlakeGenerator(worker_id=42, layout=layout)
        unique_id = flake_generator.generate()

        flake_decoder = decoder_factory(layout=layout)
        decoded_id = flake_decoder.decode(unique_id)
        assert decoded_id.worker_id == 42

    @pytest.mark.parametrize("layout", [LAYOUT_TWS, LAYOUT_TSW])
    def test_generate_returns_correct_sequence(
        self, layout: int, decoder_factory: Callable[..., FlakeDecoder]
    ) -> None:
        """
        Test that the generate method returns an identifier with the correct
        sequence.
        """
        flake_generator = FlakeGenerator(layout=layout)
        unique_id = flake_generator.generate()

        flake_decoder = decoder_factory(layout=layout)
        decoded_id = flake_decoder.decode(unique_id)
        assert decoded_id.sequence == 0

    def test_generate_increments_sequence_when_same_timestamp(
        self,
        sequence_clock_factory: Callable[..., SequenceClock],
        decoder_factory: Callable[..., FlakeDecoder],
    ) -> None:
        """
        Test that the generate method increments the sequence when the
        timestamp is the same.
        """
        clock_func = sequence_clock_factory(to_ns(milliseconds=8))
        flake_generator = FlakeGenerator(epoch_ms=0, clock_func=clock_func)

        first_id = flake_generator.generate()
        second_id = flake_generator.generate()

        flake_decoder = decoder_factory()
        decoded_first_id = flake_decoder.decode(first_id)
        assert decoded_first_id.sequence == 0

        decoded_second_id = flake_decoder.decode(second_id)
        assert decoded_second_id.sequence == 1

    def test_generate_resets_sequence_when_different_timestamp(
        self,
        sequence_clock_factory: Callable[..., SequenceClock],
        decoder_factory: Callable[..., FlakeDecoder],
    ) -> None:
        """
        Test that the generate method resets the sequence when the timestamp is
        different.
        """
        clock_func = sequence_clock_factory(
            to_ns(milliseconds=5), to_ns(milliseconds=5), to_ns(milliseconds=6)
        )
        flake_generator = FlakeGenerator(epoch_ms=0, clock_func=clock_func)

        flake_generator.generate()
        second_id = flake_generator.generate()
        third_id = flake_generator.generate()

        flake_decoder = decoder_factory()
        decoded_second_id = flake_decoder.decode(second_id)
        assert decoded_second_id.sequence == 1

        decoded_third_id = flake_decoder.decode(third_id)
        assert decoded_third_id.sequence == 0

    def test_generate_waits_for_next_timestamp_when_sequence_overflows(
        self,
        sequence_clock_factory: Callable[..., SequenceClock],
        decoder_factory: Callable[..., FlakeDecoder],
    ) -> None:
        """
        Test that the generate method waits for the next timestamp when the
        sequence overflows.
        """
        clock_func = sequence_clock_factory(
            to_ns(milliseconds=5),
            to_ns(milliseconds=5),
            to_ns(milliseconds=5),
            to_ns(milliseconds=5),
            to_ns(milliseconds=6),
        )
        flake_generator = FlakeGenerator(
            timestamp_bits=61,
            worker_bits=1,
            sequence_bits=1,
            epoch_ms=0,
            clock_func=clock_func,
        )

        first_id = flake_generator.generate()
        second_id = flake_generator.generate()
        overflow_id = flake_generator.generate()

        flake_decoder = decoder_factory(worker_bits=1, sequence_bits=1)
        decoded_first_id = flake_decoder.decode(first_id)
        assert decoded_first_id.timestamp == 5
        assert decoded_first_id.sequence == 0

        decoded_second_id = flake_decoder.decode(second_id)
        assert decoded_second_id.timestamp == 5
        assert decoded_second_id.sequence == 1

        decoded_overflow_id = flake_decoder.decode(overflow_id)
        assert decoded_overflow_id.timestamp == 6
        assert decoded_overflow_id.sequence == 0

    def test_generate_raises_time_read_error_when_clock_is_before_epoch_ms(
        self, sequence_clock_factory: Callable[..., SequenceClock]
    ) -> None:
        """
        Test that the generate method raises a TimeReadError when the clock is
        before the configured epoch.
        """
        clock_func = sequence_clock_factory(to_ns(milliseconds=99))
        flake_generator = FlakeGenerator(epoch_ms=100, clock_func=clock_func)

        with pytest.raises(
            TimeReadError, match="Failed to retrieve timestamp"
        ) as excinfo:
            flake_generator.generate()

        cause = excinfo.value.__cause__
        assert isinstance(cause, TimeBeforeEpochError)
        assert str(cause) == "Current timestamp is before the configured epoch"

    def test_generate_raises_clock_rollback_error_when_clock_moves_backwards(
        self, sequence_clock_factory: Callable[..., SequenceClock]
    ) -> None:
        """
        Test that the generate method raises a ClockRollbackError when the
        clock moves backwards.
        """
        clock_func = sequence_clock_factory(
            to_ns(milliseconds=10), to_ns(milliseconds=9)
        )
        flake_generator = FlakeGenerator(epoch_ms=0, clock_func=clock_func)

        flake_generator.generate()

        with pytest.raises(ClockRollbackError, match="Clock moved backwards"):
            flake_generator.generate()

    def test_generate_raises_time_overflow_error_when_timestamp_overflows(
        self, sequence_clock_factory: Callable[..., SequenceClock]
    ) -> None:
        """
        Test that the generate method raises a TimeOverflowError when the
        timestamp exceeds the representable range.
        """
        clock_func = sequence_clock_factory(to_ns(milliseconds=2))
        flake_generator = FlakeGenerator(
            epoch_ms=0,
            timestamp_bits=1,
            worker_bits=1,
            sequence_bits=61,
            time_unit_ns=1,
            clock_func=clock_func,
        )

        with pytest.raises(
            TimeOverflowError,
            match="Timestamp exceeded the maximum allowed value",
        ):
            flake_generator.generate()

    def test_generate_uses_custom_clock_func_when_provided(
        self, decoder_factory: Callable[..., FlakeDecoder]
    ) -> None:
        """
        Test that the generate method uses a custom clock function when it is
        provided at initialization.
        """

        def custom_clock() -> int:
            return to_ns(milliseconds=1)

        flake_generator = FlakeGenerator(epoch_ms=0, clock_func=custom_clock)
        unique_id = flake_generator.generate()

        flake_decoder = decoder_factory()
        decoded_id = flake_decoder.decode(unique_id)
        assert decoded_id.timestamp == 1


class TestFlakeGeneratorGenerateThreadSafety:
    """Thread-safety stress tests for FlakeGenerator.generate."""

    @staticmethod
    def _generate_ids_concurrently(
        *,
        flake_generator: FlakeGenerator,
        num_threads: int,
        ids_per_thread: int,
    ) -> list[int]:
        """Generate IDs from many threads sharing one generator instance."""
        start_barrier = Barrier(num_threads)

        def generate_ids() -> list[int]:
            start_barrier.wait()
            return [flake_generator.generate() for _ in range(ids_per_thread)]

        with ThreadPoolExecutor(max_workers=num_threads) as executor:
            generated_batches = list(
                executor.map(lambda _: generate_ids(), range(num_threads))
            )

        return [
            unique_id
            for generated_batch in generated_batches
            for unique_id in generated_batch
        ]

    def test_generate_has_no_collisions_under_high_contention(self) -> None:
        """
        Test that the generate method has no collisions under high contention.
        """
        num_threads = 16
        ids_per_thread = 10_000
        flake_generator = FlakeGenerator(worker_id=1, epoch_ms=0)
        generated_ids = self._generate_ids_concurrently(
            flake_generator=flake_generator,
            num_threads=num_threads,
            ids_per_thread=ids_per_thread,
        )

        assert len(generated_ids) == num_threads * ids_per_thread
        assert len(set(generated_ids)) == len(generated_ids)

    @pytest.mark.parametrize("run_index", range(5))
    def test_generate_has_no_collisions_across_repeated_contention_runs(
        self, run_index: int
    ) -> None:
        """
        Test that the generate method has no collisions across repeated
        contention runs.
        """
        num_threads = 12
        ids_per_thread = 5_000
        flake_generator = FlakeGenerator(worker_id=1, epoch_ms=0)
        generated_ids = self._generate_ids_concurrently(
            flake_generator=flake_generator,
            num_threads=num_threads,
            ids_per_thread=ids_per_thread,
        )

        assert len(generated_ids) == num_threads * ids_per_thread
        assert len(set(generated_ids)) == len(generated_ids), (
            f"Collision detected on run {run_index}"
        )

    def test_generate_produces_valid_parts_under_contention_with_overflow(
        self,
        burst_clock_factory: Callable[..., BurstClock],
        decoder_factory: Callable[..., FlakeDecoder],
    ) -> None:
        """
        Test that the generate method produces valid parts under contention
        while forcing sequence overflow.
        """
        num_threads = 8
        ids_per_thread = 500
        worker_bits = 2
        sequence_bits = 2
        max_sequence = (1 << sequence_bits) - 1
        total_ids = num_threads * ids_per_thread
        burst_size = max_sequence + 1
        clock_func = burst_clock_factory(burst_size)
        flake_generator = FlakeGenerator(
            worker_id=1,
            epoch_ms=0,
            timestamp_bits=59,
            worker_bits=worker_bits,
            sequence_bits=sequence_bits,
            clock_func=clock_func,
        )

        generated_ids = self._generate_ids_concurrently(
            flake_generator=flake_generator,
            num_threads=num_threads,
            ids_per_thread=ids_per_thread,
        )

        assert len(generated_ids) == total_ids
        assert len(set(generated_ids)) == total_ids

        flake_decoder = decoder_factory(
            worker_bits=worker_bits, sequence_bits=sequence_bits
        )
        decoded_ids = [flake_decoder.decode(uid) for uid in generated_ids]

        per_timestamp_sequences: dict[int, set[int]] = {}
        for decoded in decoded_ids:
            assert decoded.worker_id == 1
            assert 0 <= decoded.sequence <= max_sequence
            per_timestamp_sequences.setdefault(decoded.timestamp, set()).add(
                decoded.sequence
            )

        # Ensure sequence rollover happened at least once
        assert any(
            len(sequences) == max_sequence + 1
            for sequences in per_timestamp_sequences.values()
        )
