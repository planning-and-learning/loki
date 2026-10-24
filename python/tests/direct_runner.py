# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

from collections.abc import Callable
from contextlib import ExitStack
from inspect import Parameter, signature
from pathlib import Path
from tempfile import TemporaryDirectory


def _call_test(name: str, test: Callable[..., object]) -> None:
    kwargs: dict[str, Path] = {}
    with ExitStack() as stack:
        for parameter in signature(test).parameters.values():
            if parameter.name == "tmp_path" and parameter.kind in (
                Parameter.POSITIONAL_OR_KEYWORD,
                Parameter.KEYWORD_ONLY,
            ):
                tmp_dir = stack.enter_context(TemporaryDirectory(prefix=f"{name}-"))
                kwargs[parameter.name] = Path(tmp_dir)
                continue
            if parameter.default is not Parameter.empty:
                continue
            msg = f"{name} requires unsupported parameter {parameter.name!r}"
            raise TypeError(msg)

        test(**kwargs)


def run_tests(module_globals: dict[str, object]) -> None:
    tests: list[tuple[str, Callable[..., object]]] = [
        (name, value)
        for name, value in sorted(module_globals.items(), key=lambda item: item[0])
        if name.startswith("test_") and callable(value)
    ]
    if not tests:
        msg = "no tests discovered"
        raise RuntimeError(msg)

    for name, test in tests:
        try:
            _call_test(name, test)
        except Exception as error:
            msg = f"{name} failed"
            raise RuntimeError(msg) from error
