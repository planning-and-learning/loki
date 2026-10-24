# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

from importlib import import_module

from pypddl import formalism as _formalism

# _downstream_loki is a compiled nanobind extension built during the downstream test; it does not
# exist at type-check time, so resolve it at runtime (attributes type as Any).
_downstream_loki = import_module("._downstream_loki", __package__)
multiply = _downstream_loki.multiply
parses_domain = _downstream_loki.parses_domain


def describe_pypddl_imports(domain_source: str) -> dict[str, str]:
    parser = _formalism.Parser(domain_source, _formalism.ParserOptions())
    return {
        "parser": type(parser).__name__,
        "domain_name": parser.domain().get_name(),
    }
