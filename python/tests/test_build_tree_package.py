# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

from importlib.metadata import version
from importlib.util import find_spec
from pathlib import Path

import pypddl


def test_build_tree_package_exposes_current_extension_and_version() -> None:
    package_dir = Path(pypddl.__file__).parent

    assert package_dir.name == "pypddl"
    assert (package_dir / "_pypddl").is_file() or any(package_dir.glob("_pypddl*.so"))
    assert not (package_dir / "_pypddl").is_dir()
    native_spec = find_spec("pypddl._pypddl")
    assert native_spec is not None
    assert native_spec.origin is not None
    assert Path(native_spec.origin).parent == package_dir
    assert pypddl.__version__ == version("pypddl")


if __name__ == "__main__":
    from direct_runner import run_tests

    run_tests(globals())
