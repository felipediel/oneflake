"""Utilities for the tests."""


def to_ns(
    *,
    days: int = 0,
    seconds: int = 0,
    microseconds: int = 0,
    milliseconds: int = 0,
    minutes: int = 0,
    hours: int = 0,
    weeks: int = 0,
) -> int:
    """Convert a duration to a timestamp in nanoseconds."""
    seconds = (
        weeks * 604_800
        + days * 86_400
        + hours * 3_600
        + minutes * 60
        + seconds
    )
    return (
        seconds * 1_000_000_000
        + milliseconds * 1_000_000
        + microseconds * 1_000
    )
