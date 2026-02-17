"""Tests for FlakeDecoder."""

from typing import Callable

import pytest

from oneflake import (
    FlakeDecoder,
    InvalidBitWidthError,
    InvalidLayoutError,
)

from tests.common.constants import (
    DEFAULT_EPOCH_MS,
    LAYOUT_TSW,
    LAYOUT_TWS,
    TIME_UNITS_NS,
    UNIX_EPOCH_MS,
)
from tests.common.factories import FlakeEncoder
from tests.common.utils import to_ns


class TestFlakeDecoderInit:
    """Tests for the FlakeDecoder.__init__ method."""

    def test_init_sets_defaults_when_no_arguments_are_provided(self) -> None:
        """
        Test that the initialization method sets the defaults when no arguments
        are provided.
        """
        decoder = FlakeDecoder()

        assert decoder.layout == LAYOUT_TWS
        assert decoder.timestamp_bits == 41
        assert decoder.worker_bits == 10
        assert decoder.sequence_bits == 12

    def test_init_sets_custom_values_when_arguments_are_provided(self) -> None:
        """
        Test that the initialization method sets custom values when arguments
        are provided.
        """
        decoder = FlakeDecoder(
            layout=LAYOUT_TSW,
            timestamp_bits=50,
            worker_bits=8,
            sequence_bits=5,
        )

        assert decoder.layout == LAYOUT_TSW
        assert decoder.timestamp_bits == 50
        assert decoder.worker_bits == 8
        assert decoder.sequence_bits == 5

    def test_init_raises_type_error_when_keyword_only_args_are_passed_positionally(
        self,
    ) -> None:
        """
        Test that the initialization method raises a TypeError when
        keyword-only arguments are passed positionally.
        """
        with pytest.raises(TypeError):
            FlakeDecoder(LAYOUT_TSW)  # ty: ignore[too-many-positional-arguments]

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
            FlakeDecoder(layout=layout)

    def test_init_raises_overflow_error_when_layout_is_too_large_to_convert(
        self,
    ) -> None:
        """
        Test that the initialization method raises an OverflowError when the
        layout argument is too large to convert to an integer.
        """
        with pytest.raises(OverflowError):
            FlakeDecoder(layout=2**128)

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
            FlakeDecoder(timestamp_bits=timestamp_bits)

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
            FlakeDecoder(worker_bits=worker_bits)

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
            FlakeDecoder(sequence_bits=sequence_bits)

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
            FlakeDecoder(
                timestamp_bits=timestamp_bits,
                worker_bits=worker_bits,
                sequence_bits=sequence_bits,
            )


class TestFlakeDecoderDecode:
    """Tests for the FlakeDecoder.decode method."""

    @pytest.mark.parametrize("layout", [LAYOUT_TWS, LAYOUT_TSW])
    def test_decode_returns_named_tuple_like_result(
        self,
        layout: int,
        encoder_factory: Callable[..., FlakeEncoder],
    ) -> None:
        """
        Test that the decode method returns a tuple-compatible named result.
        """
        flake_encoder = encoder_factory(layout=layout, epoch_ms=0)
        unique_id = flake_encoder.encode(
            timestamp_ns=to_ns(milliseconds=3), sequence=7
        )
        decoder = FlakeDecoder(layout=layout)

        decoded_id = decoder.decode(unique_id)
        timestamp, worker_id, sequence = decoded_id

        assert isinstance(decoded_id, tuple)
        assert decoded_id.timestamp == timestamp
        assert decoded_id.worker_id == worker_id
        assert decoded_id.sequence == sequence

    @pytest.mark.parametrize("layout", [LAYOUT_TWS, LAYOUT_TSW])
    @pytest.mark.parametrize("epoch_ms", [UNIX_EPOCH_MS, DEFAULT_EPOCH_MS])
    @pytest.mark.parametrize("relative_timestamp_ms", [0, 1, 2])
    @pytest.mark.parametrize("time_unit_ns", TIME_UNITS_NS)
    def test_decode_returns_correct_timestamp(
        self,
        layout: int,
        epoch_ms: int,
        relative_timestamp_ms: int,
        time_unit_ns: int,
        encoder_factory: Callable[..., FlakeEncoder],
    ) -> None:
        """
        Test that the decode method returns the correct timestamp.
        """
        epoch_ns = to_ns(milliseconds=epoch_ms)
        relative_timestamp_ns = to_ns(milliseconds=relative_timestamp_ms)
        timestamp_ns = epoch_ns + relative_timestamp_ns
        flake_encoder = encoder_factory(
            layout=layout,
            epoch_ms=epoch_ms,
            time_unit_ns=time_unit_ns,
        )
        unique_id = flake_encoder.encode(timestamp_ns=timestamp_ns, sequence=0)
        decoder = FlakeDecoder(layout=layout)

        decoded_id = decoder.decode(unique_id)

        expected_timestamp = (
            timestamp_ns // time_unit_ns - epoch_ns // time_unit_ns
        )
        assert decoded_id[0] == expected_timestamp

    @pytest.mark.parametrize("layout", [LAYOUT_TWS, LAYOUT_TSW])
    def test_decode_returns_correct_worker_id(
        self, layout: int, encoder_factory: Callable[..., FlakeEncoder]
    ) -> None:
        """
        Test that the decode method returns the correct worker ID.
        """
        flake_encoder = encoder_factory(
            layout=layout, worker_id=42, epoch_ms=0
        )
        unique_id = flake_encoder.encode(
            timestamp_ns=to_ns(milliseconds=3), sequence=0
        )

        flake_decoder = FlakeDecoder(layout=layout)
        decoded_id = flake_decoder.decode(unique_id)
        assert decoded_id.worker_id == 42

    @pytest.mark.parametrize("layout", [LAYOUT_TWS, LAYOUT_TSW])
    def test_decode_returns_correct_sequence(
        self, layout: int, encoder_factory: Callable[..., FlakeEncoder]
    ) -> None:
        """
        Test that the decode method returns the correct sequence.
        """
        flake_encoder = encoder_factory(layout=layout, epoch_ms=0)
        unique_id = flake_encoder.encode(
            timestamp_ns=to_ns(milliseconds=3), sequence=0
        )

        flake_decoder = FlakeDecoder(layout=layout)
        decoded_id = flake_decoder.decode(unique_id)
        assert decoded_id.sequence == 0
