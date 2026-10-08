import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

import pypddl
import pyyggdrasil
import pytest


DOWNSTREAM_PACKAGE_DIR = Path(__file__).resolve().parent / "minimal_downstream_package"

DOMAIN_SOURCE = (
    Path(__file__).resolve().parents[3] / "tests" / "fixtures" / "facade" / "domain.pddl"
).read_text(encoding="utf-8")


def test_downstream_python_binding_links_installed_loki(tmp_path: Path) -> None:
    cmake = shutil.which("cmake")
    if cmake is None:
        pytest.skip("cmake is required for the downstream binding smoke test")

    pypddl_prefix = Path(pypddl.native_prefix())
    pyyggdrasil_prefix = Path(pyyggdrasil.native_prefix())
    dependency_library_dirs = [
        pypddl_prefix / "lib",
        pyyggdrasil_prefix / "lib",
    ]

    if not list((pypddl_prefix / "lib").glob("libloki_parsers.*")):
        pytest.skip("pypddl was not installed with a shared loki::parsers library")

    loki_cmake_dir = pypddl_prefix / "lib" / "cmake" / "loki"
    if not (loki_cmake_dir / "lokiConfig.cmake").exists():
        pytest.skip("pypddl was not installed with Loki CMake package files")

    project_dir = tmp_path / "minimal_downstream_package"
    shutil.copytree(DOWNSTREAM_PACKAGE_DIR, project_dir)

    build_dir = tmp_path / "build"
    subprocess.run(
        [
            cmake,
            "-S",
            str(project_dir),
            "-B",
            str(build_dir),
            f"-DCMAKE_PREFIX_PATH={pypddl_prefix};{pyyggdrasil_prefix}",
            f"-Dloki_DIR={loki_cmake_dir}",
            f"-DPython_EXECUTABLE={sys.executable}",
            f"-DDOWNSTREAM_RUNTIME_LIBRARY_DIRS={';'.join(str(path) for path in dependency_library_dirs)}",
        ],
        check=True,
    )
    subprocess.run([cmake, "--build", str(build_dir), "-j2"], check=True)

    env = os.environ.copy()
    env["PYTHONPATH"] = str(project_dir / "src") + os.pathsep + env.get("PYTHONPATH", "")
    runtime_library_path = os.pathsep.join(str(path) for path in dependency_library_dirs)
    env["LD_LIBRARY_PATH"] = runtime_library_path + os.pathsep + env.get("LD_LIBRARY_PATH", "")
    env["DYLD_LIBRARY_PATH"] = runtime_library_path + os.pathsep + env.get("DYLD_LIBRARY_PATH", "")

    result = subprocess.run(
        [
            sys.executable,
            "-c",
            (
                "import json, downstream_loki_user; "
                f"source = {DOMAIN_SOURCE!r}; "
                "print(json.dumps({"
                "'parses': downstream_loki_user.parses_domain(source), "
                "'product': downstream_loki_user.multiply(6, 7), "
                "'pypddl': downstream_loki_user.describe_pypddl_imports(source)"
                "}))"
            ),
        ],
        check=True,
        capture_output=True,
        text=True,
        env=env,
    )

    payload = json.loads(result.stdout)
    assert payload == {
        "parses": True,
        "product": 42,
        "pypddl": {
            "parser": "Parser",
            "domain_name": "facade",
        },
    }
