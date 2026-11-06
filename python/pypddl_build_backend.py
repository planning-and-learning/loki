# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later
"""PEP 517 backend for the pypddl wheel; machinery lives in pybrokkr.build_backend."""

from pybrokkr.build_backend import ProviderBackend

ProviderBackend(
    package="pypddl",
    providers=("pyyggdrasil", "pyurd"),
    prefix_defines={"pyyggdrasil": "YGGDRASIL_NATIVE_PREFIX", "pyurd": "URD_NATIVE_PREFIX"},
    jobs_env="LOKI_JOBS",
    strip_env="LOKI_STRIP_WHEEL",
).install_hooks(globals())
