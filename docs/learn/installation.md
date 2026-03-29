# Installation

## Requirements

- Python `>= 3.8`
- CPython-compatible build environment

## Install from PyPI

=== "uv"

    ```bash
    uv add oneflake
    ```

=== "pip"

    ```bash
    pip install oneflake
    ```

## Install from source

```bash
git clone https://github.com/felipediel/oneflake
cd oneflake
uv sync
```

## Build and preview docs locally

This project documentation is powered by `mkdocs-material`.

```bash
uv sync
uv run mkdocs serve
```

Then open the local URL shown in your terminal (usually
`http://localhost:8000`).
