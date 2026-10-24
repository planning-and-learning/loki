# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""Python bindings of loki, the PDDL parser and translator, and the paths of its native build."""

import tomllib as _tomllib
from importlib import import_module as _import_module
from importlib.metadata import PackageNotFoundError, version
from pathlib import Path

# Load native dependency packages before this package loads native extensions.
_import_module("pyyggdrasil")

from pypddl import formalism as formalism


def source_version() -> str:
    """Return the version in the nearest pyproject.toml, for source checkouts without installed metadata."""
    for parent in Path(__file__).resolve().parents:
        pyproject = parent / "pyproject.toml"
        if not pyproject.exists():
            continue

        project_version = _tomllib.loads(pyproject.read_text(encoding="utf-8")).get("project", {}).get("version")
        if project_version is not None:
            return project_version

    return "0.0.0"


try:
    __version__ = version("pypddl")
except PackageNotFoundError:
    __version__ = source_version()


def native_prefix() -> Path:
    """Return the prefix of loki's bundled native build (headers, libraries, CMake files)."""
    package_dir = Path(__file__).resolve().parent
    native_dir = package_dir / "native"
    if (native_dir / "include" / "loki").is_dir():
        return native_dir
    for parent in package_dir.parents:
        if (parent / "include" / "loki").is_dir() and any(
            (parent / name / "cmake" / "loki").is_dir() for name in ("lib", "lib64")
        ):
            return parent
    return native_dir


def native_include_dir() -> Path:
    """Return the directory of loki's bundled C++ headers."""
    return native_prefix() / "include"


def native_lib_dir() -> Path:
    """Return the directory of loki's bundled libraries."""
    native_dir = native_prefix()
    for lib_dir_name in ("lib", "lib64"):
        lib_dir = native_dir / lib_dir_name
        if lib_dir.is_dir():
            return lib_dir
    return native_dir / "lib"


def cmake_prefix() -> Path:
    """Return the prefix to put on CMAKE_PREFIX_PATH to find loki via find_package."""
    return native_prefix()


def cmake_dir() -> Path:
    """Return the directory containing lokiConfig.cmake."""
    prefix = native_prefix()
    for lib_dir_name in ("lib", "lib64"):
        candidate = prefix / lib_dir_name / "cmake" / "loki"
        if (candidate / "lokiConfig.cmake").is_file():
            return candidate

    msg = f"lokiConfig.cmake not found under {prefix}; the installed pypddl is too old or incomplete."
    raise FileNotFoundError(msg)
