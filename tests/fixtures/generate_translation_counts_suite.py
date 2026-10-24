# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later
"""Regenerate translation_counts_suite.json.

Runs every parser-suite fixture through each parser/translator configuration and records
entity counts of the translated domain and task.

Usage: uv run python tests/fixtures/generate_translation_counts_suite.py
"""

import json
from pathlib import Path
from typing import Any, TypeVar

import pypddl_datasets

from pypddl import formalism as pypddl

ROOT = Path(__file__).resolve().parents[2]

_Options = TypeVar("_Options")

# strict mode is exercised by unit tests only: the benchmark fixtures are not strict-clean.
CONFIGURATIONS: list[dict[str, dict[str, bool]]] = [
    {
        "parser_options": {"add_action_costs": add_action_costs},
        "options": {
            "compile_typing": compile_typing,
            "compile_conditional_effects": compile_conditional_effects,
            "materialize_equality": materialize_equality,
        },
    }
    for add_action_costs, compile_typing, compile_conditional_effects, materialize_equality in [
        (False, False, False, False),
        (False, True, False, True),
        (False, False, True, False),
        (False, True, True, True),
        (True, False, False, False),
        (True, True, True, True),
    ]
]


def make_options(cls: type[_Options], config: dict[str, bool]) -> _Options:
    """Options of type cls with the given settings."""
    options = cls()
    for key, value in config.items():
        setattr(options, key, value)
    return options


def main() -> None:
    """Write the translation counts suite."""
    parser_suite: dict[str, Any] = json.loads((ROOT / "tests/fixtures/parser_counts_suite.json").read_text())

    prefix = pypddl_datasets.data_root()

    cases: list[dict[str, Any]] = []
    for case in parser_suite["cases"]:
        configurations: list[dict[str, Any]] = []
        for config in CONFIGURATIONS:
            parser_options = make_options(pypddl.ParserOptions, config["parser_options"])
            translator_options = make_options(pypddl.TranslatorOptions, config["options"])
            parser = pypddl.Parser(prefix / case["domain_file"], parser_options)
            task = parser.parse_task(prefix / case["task_file"])
            domain_translation = pypddl.translate_domain(parser.domain(), translator_options)
            translated_domain = domain_translation.translated_domain
            translated_task = pypddl.translate_task(task, domain_translation, translator_options).translated_task
            configurations.append(
                {
                    **config,
                    "translated_domain": {
                        "types": len(translated_domain.get_types()),
                        "predicates": len(translated_domain.get_predicates()),
                        "functions": len(translated_domain.get_functions()),
                        "actions": len(translated_domain.get_actions()),
                        "axioms": len(translated_domain.get_axioms()),
                    },
                    "translated_task": {
                        "objects": len(translated_task.get_objects()),
                        "predicates": len(translated_task.get_predicates()),
                        "initial_literals": len(translated_task.get_initial_literals()),
                        "initial_function_values": len(translated_task.get_initial_function_values()),
                        "actions": len(translated_task.get_domain().get_actions()),
                        "axioms": len(translated_task.get_axioms()),
                    },
                }
            )
        cases.append({"name": case["name"], "configurations": configurations})

    out = ROOT / "tests/fixtures/translation_counts_suite.json"
    out.write_text(json.dumps({"cases": cases}, indent=4) + "\n")
    print(f"Wrote {len(cases)} cases x {len(CONFIGURATIONS)} configurations to {out}")


if __name__ == "__main__":
    main()
