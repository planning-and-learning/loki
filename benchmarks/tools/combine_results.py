# Copyright (C) 2024-2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later
"""Combine Google Benchmark JSON results into one file, printed to stdout."""

import argparse
import json
from collections.abc import Sequence
from pathlib import Path
from typing import cast


def combine_json_files(files: Sequence[str]) -> dict[str, object] | None:
    """The first file's results, with the benchmarks of all files."""
    combined_data: dict[str, object] | None = None
    for file in files:
        with Path(file).open() as f:
            data = cast("dict[str, object]", json.load(f))
        if combined_data is None:
            combined_data = data
        else:
            cast("list[object]", combined_data["benchmarks"]).extend(cast("list[object]", data["benchmarks"]))
    return combined_data


def main() -> None:
    """Combine the files named on the command line."""
    parser = argparse.ArgumentParser(description="Combine JSON benchmark files.")
    parser.add_argument("files", nargs="+", help="JSON files to combine")
    args = parser.parse_args()
    print(json.dumps(combine_json_files(cast("list[str]", args.files)), indent=4))


if __name__ == "__main__":
    main()
