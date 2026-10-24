# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""``python -m pypddl``: print where the bundled native loki library is."""

import argparse

import pypddl


def main() -> None:
    """Print the requested path or version."""
    parser = argparse.ArgumentParser(
        prog="python -m pypddl",
        description="Print discovery paths for the bundled native loki library.",
    )
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "--prefix",
        action="store_true",
        help="Print the native prefix to put on CMAKE_PREFIX_PATH.",
    )
    group.add_argument(
        "--include-dir",
        action="store_true",
        help="Print the bundled C++ header include directory.",
    )
    group.add_argument(
        "--cmake-dir",
        action="store_true",
        help="Print the directory containing lokiConfig.cmake.",
    )
    group.add_argument(
        "--version",
        action="store_true",
        help="Print the pypddl version.",
    )
    args = parser.parse_args()

    if args.prefix:
        print(pypddl.cmake_prefix())
    elif args.include_dir:
        print(pypddl.native_include_dir())
    elif args.cmake_dir:
        print(pypddl.cmake_dir())
    elif args.version:
        print(pypddl.__version__)


if __name__ == "__main__":
    main()
