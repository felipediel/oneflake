#!/usr/bin/env python
"""Setup script for the oneflake package."""

from __future__ import annotations

import sysconfig
from typing import Any

from setuptools import Extension, setup

options: dict[str, Any] = {}
define_macros: list[tuple[str, str | None]] = []
py_limited_api: bool = not sysconfig.get_config_var("Py_GIL_DISABLED")
cflags: list[str] = []

if sysconfig.get_platform().startswith("win"):
    cflags.append("/utf-8")
    cflags.append("/std:c17")
    cflags.append("/Zc:preprocessor")
    cflags.append("/experimental:c11atomics")
else:
    cflags.append("-std=c17")
    cflags.append("-Wall")
    cflags.append("-Wextra")
    cflags.append("-Werror")

if py_limited_api:
    options["bdist_wheel"] = {"py_limited_api": "cp38"}
    define_macros.append(("Py_LIMITED_API", "0x03080000"))

setup(
    options=options,
    ext_modules=[
        Extension(
            "oneflake._oneflake",
            sources=[
                "src/oneflake/_oneflake.c",
                "src/oneflake/_constants.c",
                "src/oneflake/_exceptions.c",
                "src/oneflake/_validation.c",
                "src/oneflake/_generator.c",
                "src/oneflake/_decoder.c",
            ],
            define_macros=define_macros,
            extra_compile_args=cflags,
            py_limited_api=py_limited_api,
        )
    ],
)
