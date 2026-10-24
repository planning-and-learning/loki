# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import importlib.util
from importlib import import_module
from pathlib import Path
from unittest.mock import patch

import pypddl


def test_top_level_package_exports_public_helpers_and_formalism_module() -> None:
    assert isinstance(pypddl.__version__, str)
    assert pypddl.__version__ != ""
    assert callable(pypddl.native_prefix)
    assert callable(pypddl.native_include_dir)
    assert callable(pypddl.native_lib_dir)
    assert pypddl.formalism.Parser is not None
    assert pypddl.formalism.RepositoryFactory is not None


def test_source_version_reads_the_project_table(tmp_path: Path) -> None:
    (tmp_path / "pyproject.toml").write_text(
        "[tool.other]\nversion = 'unrelated'\n[project]\nversion = '1.2.3' # comment\n",
        encoding="utf-8",
    )
    with patch.object(pypddl, "__file__", str(tmp_path / "pypddl" / "__init__.py")):
        assert pypddl.source_version() == "1.2.3"


def test_public_stubs_describe_public_api() -> None:
    package_dir = Path(pypddl.__file__).parent

    package_stub = (package_dir / "__init__.pyi").read_text(encoding="utf-8")
    assert "from . import formalism as formalism" in package_stub
    assert "def native_prefix() -> Path: ..." in package_stub
    assert "__all__" not in package_stub


def test_native_prefix_layout() -> None:
    native_prefix = pypddl.native_prefix()
    native_include_dir = pypddl.native_include_dir()
    native_lib_dir = pypddl.native_lib_dir()
    package_dir = Path(pypddl.__file__).parent

    assert isinstance(native_prefix, Path)
    assert isinstance(native_include_dir, Path)
    assert isinstance(native_lib_dir, Path)
    assert native_include_dir == native_prefix / "include"
    assert native_lib_dir.name in {"lib", "lib64"}

    assert not (package_dir / "_pypddl").is_dir()

    if (native_prefix / "include" / "loki").is_dir():
        assert native_include_dir.is_dir()
        assert native_lib_dir.is_dir()
        assert (native_lib_dir / "cmake" / "loki").is_dir()
        assert (package_dir / "py.typed").is_file()
    else:
        assert (package_dir / "__init__.pyi").is_file()
        assert (package_dir / "py.typed").is_file()
        assert (package_dir / "_version.py").is_file()
        assert (package_dir / "_pypddl").is_file() or any(package_dir.glob("_pypddl*.so"))


def test_formalism_package_reexports_native_public_symbols() -> None:
    native_formalism = import_module("pypddl._pypddl.formalism")
    missing = [
        name for name in dir(native_formalism) if not name.startswith("_") and not hasattr(pypddl.formalism, name)
    ]

    assert missing == []
    assert not hasattr(pypddl.formalism, "__all__")


def test_programmatic_gripper_example_uses_public_api() -> None:
    example_path = Path(__file__).resolve().parents[1] / "example" / "programmatic_gripper.py"
    spec = importlib.util.spec_from_file_location("programmatic_gripper", example_path)
    assert spec is not None
    assert spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)

    _, domain, one_ball, two_ball = module.build_gripper()

    domain_text = pypddl.formalism.format_domain(domain)
    one_ball_text = pypddl.formalism.format_task(one_ball)
    reparsed = pypddl.formalism.Parser(domain_text, pypddl.formalism.ParserOptions())
    reparsed_one_ball = reparsed.parse_task(one_ball_text)

    assert reparsed.domain().get_name() == domain.get_name()
    assert reparsed_one_ball.get_name() == one_ball.get_name()

    assert domain.get_name() == "gripper"
    assert len(domain.get_actions()) == 3
    assert len(domain.get_predicates()) == 4
    assert len(one_ball.get_objects()) == 5
    assert len(one_ball.get_initial_literals()) == 4
    assert len(two_ball.get_objects()) == 6
    assert len(two_ball.get_initial_literals()) == 5


if __name__ == "__main__":
    from direct_runner import run_tests

    run_tests(globals())
