"""Tests for the oneflake public module API."""

from typing import Any

import pytest

import oneflake

from tests.common.constants import (
    DEFAULT_EPOCH_MS,
    DEFAULT_LAYOUT,
    DEFAULT_SEQUENCE_BITS,
    DEFAULT_TIME_UNIT_NS,
    DEFAULT_TIMESTAMP_BITS,
    DEFAULT_WORKER_BITS,
    DEFAULT_WORKER_ID,
    LAYOUT_TSW,
    LAYOUT_TWS,
    MAX_EPOCH_MS,
    MIN_EPOCH_MS,
    UNIX_EPOCH_MS,
)


class TestPublicAttributes:
    """Tests for public attributes of the oneflake module."""

    ALLOWED_PUBLIC_ATTRIBUTES = {
        "DEFAULT_WORKER_ID",
        "DEFAULT_TIMESTAMP_BITS",
        "DEFAULT_WORKER_BITS",
        "DEFAULT_SEQUENCE_BITS",
        "DEFAULT_TIME_UNIT_NS",
        "DEFAULT_EPOCH_MS",
        "DEFAULT_LAYOUT",
        "LAYOUT_TSW",
        "LAYOUT_TWS",
        "MAX_EPOCH_MS",
        "MIN_EPOCH_MS",
        "UNIX_EPOCH_MS",
        "FlakeDecoder",
        "FlakeGenerator",
        "FlakeParts",
        "OneflakeException",
        "ConfigurationError",
        "ClockError",
        "DecodeError",
        "InvalidLayoutError",
        "InvalidBitWidthError",
        "InvalidWorkerError",
        "InvalidTimeUnitError",
        "InvalidEpochError",
        "InvalidClockFunctionError",
        "TimeBeforeEpochError",
        "TimeReadError",
        "ClockRollbackError",
        "TimeOverflowError",
        "LayoutDecodeError",
    }

    @pytest.mark.parametrize(
        "attribute, value",
        [
            ("DEFAULT_WORKER_ID", DEFAULT_WORKER_ID),
            ("DEFAULT_TIMESTAMP_BITS", DEFAULT_TIMESTAMP_BITS),
            ("DEFAULT_WORKER_BITS", DEFAULT_WORKER_BITS),
            ("DEFAULT_SEQUENCE_BITS", DEFAULT_SEQUENCE_BITS),
            ("DEFAULT_TIME_UNIT_NS", DEFAULT_TIME_UNIT_NS),
            ("DEFAULT_EPOCH_MS", DEFAULT_EPOCH_MS),
            ("DEFAULT_LAYOUT", DEFAULT_LAYOUT),
            ("LAYOUT_TSW", LAYOUT_TSW),
            ("LAYOUT_TWS", LAYOUT_TWS),
            ("MAX_EPOCH_MS", MAX_EPOCH_MS),
            ("MIN_EPOCH_MS", MIN_EPOCH_MS),
            ("UNIX_EPOCH_MS", UNIX_EPOCH_MS),
        ],
    )
    def test_default_constants_are_exposed_with_expected_values(
        self, attribute: str, value: Any
    ) -> None:
        """
        Test that default constants are exposed with the expected values.
        """
        assert hasattr(oneflake, attribute)
        assert getattr(oneflake, attribute) == value

    @pytest.mark.parametrize(
        "cls_name",
        [
            "FlakeDecoder",
            "FlakeGenerator",
            "FlakeParts",
            "OneflakeException",
            "ConfigurationError",
            "ClockError",
            "DecodeError",
            "InvalidLayoutError",
            "InvalidBitWidthError",
            "InvalidWorkerError",
            "InvalidTimeUnitError",
            "InvalidEpochError",
            "InvalidClockFunctionError",
            "TimeBeforeEpochError",
            "TimeReadError",
            "ClockRollbackError",
            "TimeOverflowError",
            "LayoutDecodeError",
        ],
    )
    def test_public_classes_are_exposed_with_expected_type(
        self, cls_name: str
    ) -> None:
        """
        Test that public classes are exposed with the expected type.
        """
        assert hasattr(oneflake, cls_name)
        cls = getattr(oneflake, cls_name)
        assert isinstance(cls, type), f"{cls_name} should be a class type"

    def test_no_extra_public_attributes_are_exposed(self) -> None:
        """
        Test that no extra public attributes are exposed.
        """
        public_attrs = {
            name for name in dir(oneflake) if not name.startswith("_")
        }
        extra = public_attrs - self.ALLOWED_PUBLIC_ATTRIBUTES
        assert not extra, f"Unexpected public attributes: {extra}"

    def test_exception_hierarchy_matches_public_contract(self) -> None:
        """
        Test that custom exceptions inherit according to the documented
        hierarchy.
        """
        assert issubclass(
            oneflake.ConfigurationError, oneflake.OneflakeException
        )
        assert issubclass(oneflake.ClockError, oneflake.OneflakeException)
        assert issubclass(oneflake.DecodeError, oneflake.OneflakeException)

        assert issubclass(
            oneflake.InvalidLayoutError, oneflake.ConfigurationError
        )
        assert issubclass(
            oneflake.InvalidBitWidthError, oneflake.ConfigurationError
        )
        assert issubclass(
            oneflake.InvalidWorkerError, oneflake.ConfigurationError
        )
        assert issubclass(
            oneflake.InvalidTimeUnitError, oneflake.ConfigurationError
        )
        assert issubclass(
            oneflake.InvalidEpochError, oneflake.ConfigurationError
        )
        assert issubclass(
            oneflake.InvalidClockFunctionError, oneflake.ConfigurationError
        )

        assert issubclass(oneflake.TimeBeforeEpochError, oneflake.ClockError)
        assert issubclass(oneflake.TimeReadError, oneflake.ClockError)
        assert issubclass(oneflake.ClockRollbackError, oneflake.ClockError)
        assert issubclass(oneflake.TimeOverflowError, oneflake.ClockError)

        assert issubclass(oneflake.LayoutDecodeError, oneflake.DecodeError)
