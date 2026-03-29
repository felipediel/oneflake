# Release Notes

This page tracks notable changes across Oneflake releases.

## Unreleased

### Documentation

- Introduced a product-style documentation architecture with top-level sections: **Introduction**, **Learn**, **Reference**, **Resources**, **About**, and **Release Notes**.

## v0.1.0 (2026-03-16)

### New Features

- **ID generator:** Generate globally unique, compact, time-sortable 64-bit IDs.
- **ID decoder:** Decode IDs into timestamp, worker ID, and sequence components.
- **Configurable layout:** Set timestamp bits, worker bits, and sequence bits; choose field order via timestamp-worker-sequence or timestamp-sequence-worker layouts.
- **Custom time:** Configurable epoch and time unit for timestamp precision.
- **Lock-free C core:** High-throughput, thread-safe ID generation without mutex contention.
