# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later
"""Regenerate parser_counts_suite.json from all packaged benchmark problems.

Usage: uv run python tests/fixtures/generate_parser_counts_suite.py
"""

import json
from pathlib import Path
from typing import Any

import pypddl_datasets

from pypddl import formalism as pypddl

ROOT = Path(__file__).resolve().parents[2]
CONFIGURATIONS: list[dict[str, bool]] = [
    {"strict": strict, "add_action_costs": add_action_costs}
    for strict in (False, True)
    for add_action_costs in (False, True)
]


def discover_cases(prefix: Path) -> list[dict[str, Any]]:
    """Each domain and task of the dataset's test suites, named uniquely."""
    cases: list[dict[str, Any]] = []
    for family in ("classical", "numeric"):
        family_root = prefix / family / "tests"
        for domain_file in sorted(family_root.rglob("domain.pddl")):
            task_files = sorted(path for path in domain_file.parent.glob("*.pddl") if path.name != "domain.pddl")
            relative_dir = domain_file.parent.relative_to(family_root).as_posix()
            for task_file in task_files:
                name = f"{family}/{relative_dir}"
                if len(task_files) > 1:
                    name += f"/{task_file.stem}"
                cases.append(
                    {
                        "name": name,
                        "domain_file": domain_file.relative_to(prefix).as_posix(),
                        "task_file": task_file.relative_to(prefix).as_posix(),
                    }
                )
    if not cases:
        msg = f"no test cases under {prefix}"
        raise RuntimeError(msg)
    if len({case["name"] for case in cases}) != len(cases):
        msg = "test case names are not unique"
        raise RuntimeError(msg)
    return cases


def make_options(config: dict[str, bool]) -> pypddl.ParserOptions:
    """Parser options with the given settings."""
    options = pypddl.ParserOptions()
    for key, value in config.items():
        setattr(options, key, value)
    return options


def domain_counts(domain: pypddl.Domain) -> dict[str, int]:
    """How many of each part the domain has."""
    return {
        "requirements": len(domain.get_requirements()),
        "types": len(domain.get_types()),
        "constants": len(domain.get_constants()),
        "predicates": len(domain.get_predicates()),
        "functions": len(domain.get_functions()),
        "actions": len(domain.get_actions()),
        "axioms": len(domain.get_axioms()),
    }


def task_counts(task: pypddl.Task) -> dict[str, int | bool]:
    """How many of each part the task has, and whether it has a goal and a metric."""
    return {
        "requirements": len(task.get_requirements()),
        "objects": len(task.get_objects()),
        "initial_literals": len(task.get_initial_literals()),
        "initial_function_values": len(task.get_initial_function_values()),
        "predicates": len(task.get_predicates()),
        "axioms": len(task.get_axioms()),
        "has_goal": task.get_goal() is not None,
        "has_metric": task.get_metric() is not None,
    }


def main() -> None:
    """Write the parser counts suite."""
    prefix = Path(pypddl_datasets.data_root())
    cases = discover_cases(prefix)
    for case in cases:
        configurations: list[dict[str, Any]] = []
        for options_config in CONFIGURATIONS:
            parser = pypddl.Parser(prefix / case["domain_file"], make_options(options_config))
            domain = parser.domain()
            task = parser.parse_task(prefix / case["task_file"])
            configurations.append(
                {
                    "parser_options": options_config,
                    "domain": domain_counts(domain),
                    "task": task_counts(task),
                }
            )
        case["configurations"] = configurations

    out = ROOT / "tests/fixtures/parser_counts_suite.json"
    out.write_text(json.dumps({"cases": cases}, indent=4) + "\n")
    print(f"Wrote {len(cases)} cases x {len(CONFIGURATIONS)} configurations to {out}")


if __name__ == "__main__":
    main()
