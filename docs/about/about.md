# About Oneflake

Oneflake is a high-performance 64-bit ID generation library for distributed systems.

## Project goals

- Provide deterministic, compact, and time-sortable identifiers.
- Keep the public Python API straightforward while preserving low-level performance.
- Offer clear operational contracts for production systems.

## Design principles

- **Predictability first**: explicit field layout and decode semantics.
- **Safety by default**: validation and typed exceptions for invalid runtime states.
- **Performance with clarity**: C-backed generation engine with Python ergonomics.

## Author

Oneflake was created by [Felipe Martins Diel](https://github.com/felipediel).

## License

This project is licensed under the
[MIT License](https://github.com/felipediel/oneflake/blob/main/LICENSE).

## Repository

- Source code: <https://github.com/felipediel/oneflake>
- Package: <https://pypi.org/project/oneflake/>
