# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later
"""PEP 517 backend for the pypddl wheel; machinery lives in pyyggdrasil.build_support."""

from pyyggdrasil.build_support import ProviderBackend

ProviderBackend(
    package="pypddl",
    providers=("pyyggdrasil",),
    jobs_env="LOKI_JOBS",
    strip_env="LOKI_STRIP_WHEEL",
).install_hooks(globals())
