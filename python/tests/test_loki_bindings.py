import gc
import json
import operator
import sys
from collections.abc import Iterable
from pathlib import Path
from typing import Any, cast

from pypddl import formalism as pypddl
import pypddl_datasets
from pyyggdrasil.diagnostics import Diagnostic, format_diagnostic


ROOT = Path(__file__).resolve().parents[2]
FIXTURE_DIR = ROOT / "tests" / "fixtures"


def fixture_path(case_dir: str, file_name: str = "domain.pddl") -> Path:
    return FIXTURE_DIR / case_dir / file_name


def fixture_text(case_dir: str, file_name: str = "domain.pddl") -> str:
    return fixture_path(case_dir, file_name).read_text(encoding="utf-8")


def load_suite(file_name: str) -> dict[str, Any]:
    return cast(dict[str, Any], json.loads((FIXTURE_DIR / file_name).read_text(encoding="utf-8")))


def test_positive_fixture_suite_replays() -> None:
    suite = load_suite("suite.json")
    base = ROOT / cast(str, suite["prefix"])
    for case in cast(list[dict[str, Any]], suite["cases"]):
        parser = pypddl.Parser(base / cast(str, case["domain_file"]), pypddl.ParserOptions())
        translator_options = pypddl.TranslatorOptions()
        domain_translation = pypddl.translate_domain(parser.domain(), translator_options)
        task_file = cast(str | None, case.get("task_file"))
        if task_file is not None:
            task = parser.parse_task(base / task_file)
            pypddl.translate_task(task, domain_translation, translator_options)


def test_negative_fixture_suite_replays() -> None:
    suite = load_suite("negative_suite.json")
    base = ROOT / cast(str, suite["prefix"])
    for case in cast(list[dict[str, Any]], suite["cases"]):
        options = pypddl.ParserOptions()
        options.strict = cast(bool, case.get("strict", False))
        options.add_action_costs = False
        try:
            parser = pypddl.Parser((base / cast(str, case["domain_file"])).read_text(encoding="utf-8"), options)
            task_file = cast(str | None, case.get("task_file"))
            if task_file is not None:
                parser.parse_task((base / task_file).read_text(encoding="utf-8"))
        except pypddl.SemanticError as error:
            expected_error = cast(str, case["expected_error"])
            assert expected_error == "SemanticError" or type(error).__name__ == expected_error, case["name"]
            message = str(error)
            expected_message = cast(str | None, case.get("expected_message"))
            if expected_message is not None:
                assert expected_message in message, case["name"]
            expected_line = cast(int | None, case.get("expected_line"))
            if expected_line is not None:
                assert f"In line {expected_line}:" in message, case["name"]
            expected_column = cast(int | None, case.get("expected_column"))
            if expected_column is not None:
                assert error.diagnostic.location is not None, case["name"]
                assert error.diagnostic.location.column == expected_column, case["name"]
        else:
            raise AssertionError(f"{case['name']}: expected semantic error")


def test_parse_error_keeps_owned_diagnostic_context_and_filename() -> None:
    path = fixture_path("broken-syntax")
    try:
        pypddl.Parser(path, pypddl.ParserOptions())
    except pypddl.ParseError as error:
        assert type(error) is pypddl.ParseError
        assert isinstance(error, pypddl.SemanticError)
        diagnostic = error.diagnostic
        rendered = str(error)
        try:
            setattr(error, "diagnostic", diagnostic)
        except AttributeError:
            pass
        else:
            raise AssertionError("diagnostic must be read-only")
    else:
        raise AssertionError("expected ParseError")

    gc.collect()
    assert isinstance(diagnostic, Diagnostic)
    assert diagnostic.location is not None
    assert diagnostic.location.source.filename == str(path)
    assert diagnostic.location.source.text == path.read_text(encoding="utf-8")
    assert diagnostic.location.line == 4
    assert diagnostic.location.column == 1
    assert diagnostic.location.begin == len(diagnostic.location.source.text.encode("utf-8"))
    assert "while parsing domain definition" in diagnostic.message
    assert len(diagnostic.notes) == 1
    assert diagnostic.notes[0].message == "domain definition starts here"
    assert diagnostic.notes[0].location is not None
    assert diagnostic.notes[0].location.begin == 0
    assert rendered == format_diagnostic(diagnostic)


def test_parser_distinguishes_comments_trailing_tokens_and_missing_close() -> None:
    cases = [
        ("(define (domain sample)) ; ) not extra input\n", None),
        ("(define (domain sample)) ; comment\nextra", "extra"),
        ("(define (domain sample) ; ) not a close\n", ""),
    ]
    for source, offending_token in cases:
        try:
            pypddl.Parser(source, pypddl.ParserOptions())
        except pypddl.ParseError as error:
            assert offending_token is not None
            diagnostic = error.diagnostic
            assert diagnostic.location is not None
            expected_offset = source.index(offending_token) if offending_token else len(source)
            assert diagnostic.location.begin == expected_offset
            if offending_token:
                assert diagnostic.message == "Unexpected trailing input."
            else:
                assert "while parsing domain definition" in diagnostic.message
                assert diagnostic.notes[0].message == "domain definition starts here"
        else:
            assert offending_token is None, "invalid source unexpectedly parsed"


def test_semantic_error_keeps_typed_category_and_owned_identifier_range() -> None:
    source = "(define (domain sample) (:requirements :strips)\n (:predicates (p))\n (:action a :parameters () :precondition (missing) :effect (p)))"
    options = pypddl.ParserOptions()
    options.strict = True
    options.add_action_costs = False
    try:
        pypddl.Parser(source, options)
    except pypddl.UndefinedPredicateError as error:
        assert type(error) is pypddl.UndefinedPredicateError
        assert isinstance(error, pypddl.SemanticError)
        diagnostic = error.diagnostic
        rendered = str(error)
    else:
        raise AssertionError("expected UndefinedPredicateError")
    del source
    gc.collect()
    assert isinstance(diagnostic, Diagnostic)
    assert diagnostic.message == "Undefined predicate: missing"
    assert diagnostic.location is not None
    span = diagnostic.location
    assert span.line == 3
    assert span.source.text.encode("utf-8")[span.begin:span.end] == b"missing"
    assert rendered == format_diagnostic(diagnostic)


def test_syntax_error_belongs_to_the_deepest_identified_construct() -> None:
    cases = [
        ("(:action a :parameters bare)", "action", "(:action"),
        ("(:action a :parameters (bare))", "action parameters", "(bare"),
        ("(:action a :parameters () :precondition bare)", "action", "(:action"),
        ("(:action a :parameters () :precondition (not bare))", "not condition", "(not"),
        ("(:action a :parameters () :precondition (and (not bare)))", "not condition", "(not"),
        ("(:action a :parameters () :effect (when bare (p)))", "when effect", "(when"),
        ("(:action a :parameters () :effect (when (not bare) (p)))", "not condition", "(not"),
    ]
    for action, owner, opening in cases:
        source = f"(define (domain sample) (:predicates (p)) {action})"
        try:
            pypddl.Parser(source, pypddl.ParserOptions())
        except pypddl.ParseError as error:
            diagnostic = error.diagnostic
            assert f"while parsing {owner}" in diagnostic.message, action
            assert diagnostic.location is not None
            assert diagnostic.location.begin == source.index("bare"), action
            assert len(diagnostic.notes) == 1
            assert diagnostic.notes[0].message == f"{owner} starts here"
            assert diagnostic.notes[0].location is not None
            assert diagnostic.notes[0].location.begin == source.index(opening), action
        else:
            raise AssertionError(f"malformed construct was accepted as an atom: {action}")


def test_manually_created_exception_has_locationless_diagnostic() -> None:
    error = pypddl.MissingDomainError("no domain")
    assert isinstance(error.diagnostic, Diagnostic)
    assert error.diagnostic.message == "no domain"
    assert error.diagnostic.location is None
    assert not error.diagnostic.notes


def test_expected_grammar_subjects_are_readable() -> None:
    cases = [
        ("(define (domain))", "Expected identifier while parsing domain header"),
        ("(define (domain d) (:action a))", "Expected :parameters while parsing action"),
        ("(define (domain d) (:action a :parameters bare))", "Expected action parameters while parsing action"),
        ("(define (domain d) (:types a - (either)))", "Expected one or more type expressions while parsing either type"),
    ]
    for source, expected in cases:
        try:
            pypddl.Parser(source, pypddl.ParserOptions())
        except pypddl.ParseError as error:
            assert error.diagnostic.message == expected
        else:
            raise AssertionError(f"expected syntax error: {source}")

    parser = pypddl.Parser("(define (domain d))", pypddl.ParserOptions())
    for source, expected in [
        ("(define (problem p) (:init))", "Expected problem domain while parsing problem definition"),
        ("(define (problem p) (:domain d))", "Expected init section while parsing problem definition"),
    ]:
        try:
            parser.parse_task(source)
        except pypddl.ParseError as error:
            assert error.diagnostic.message == expected
        else:
            raise AssertionError(f"expected syntax error: {source}")


def test_parser_count_fixture_suite_replays() -> None:
    benchmark_root = pypddl_datasets.data_root()
    for case in cast(list[dict[str, Any]], load_suite("parser_counts_suite.json")["cases"]):
        for configuration in cast(list[dict[str, Any]], case["configurations"]):
            option_values = cast(dict[str, bool], configuration["parser_options"])
            options = pypddl.ParserOptions()
            options.strict = option_values["strict"]
            options.add_action_costs = option_values["add_action_costs"]
            parser = pypddl.Parser(benchmark_root / cast(str, case["domain_file"]), options)
            domain = parser.domain()
            task = parser.parse_task(benchmark_root / cast(str, case["task_file"]))

            assert {
                "requirements": len(domain.get_requirements()),
                "types": len(domain.get_types()),
                "constants": len(domain.get_constants()),
                "predicates": len(domain.get_predicates()),
                "functions": len(domain.get_functions()),
                "actions": len(domain.get_actions()),
                "axioms": len(domain.get_axioms()),
            } == configuration["domain"], case["name"]
            assert {
                "requirements": len(task.get_requirements()),
                "objects": len(task.get_objects()),
                "initial_literals": len(task.get_initial_literals()),
                "initial_function_values": len(task.get_initial_function_values()),
                "predicates": len(task.get_predicates()),
                "axioms": len(task.get_axioms()),
                "has_goal": task.get_goal() is not None,
                "has_metric": task.get_metric() is not None,
            } == configuration["task"], case["name"]


def test_translation_count_fixture_suite_replays() -> None:
    benchmark_root = pypddl_datasets.data_root()
    parser_cases = {
        cast(str, case["name"]): case
        for case in cast(list[dict[str, Any]], load_suite("parser_counts_suite.json")["cases"])
    }
    for expected in cast(list[dict[str, Any]], load_suite("translation_counts_suite.json")["cases"]):
        case = parser_cases[cast(str, expected["name"])]
        for configuration in cast(list[dict[str, Any]], expected["configurations"]):
            parser_options = pypddl.ParserOptions()
            parser_options.add_action_costs = cast(dict[str, bool], configuration["parser_options"])["add_action_costs"]
            option_values = cast(dict[str, bool], configuration["options"])
            translator_options = pypddl.TranslatorOptions()
            translator_options.compile_typing = option_values["compile_typing"]
            translator_options.compile_conditional_effects = option_values["compile_conditional_effects"]
            translator_options.materialize_equality = option_values["materialize_equality"]

            parser = pypddl.Parser(benchmark_root / cast(str, case["domain_file"]), parser_options)
            task = parser.parse_task(benchmark_root / cast(str, case["task_file"]))
            domain_translation = pypddl.translate_domain(parser.domain(), translator_options)
            domain = domain_translation.translated_domain
            translated_task = pypddl.translate_task(task, domain_translation, translator_options).translated_task

            assert {
                "types": len(domain.get_types()),
                "predicates": len(domain.get_predicates()),
                "functions": len(domain.get_functions()),
                "actions": len(domain.get_actions()),
                "axioms": len(domain.get_axioms()),
            } == configuration["translated_domain"], expected["name"]
            assert {
                "objects": len(translated_task.get_objects()),
                "predicates": len(translated_task.get_predicates()),
                "initial_literals": len(translated_task.get_initial_literals()),
                "initial_function_values": len(translated_task.get_initial_function_values()),
                "actions": len(translated_task.get_domain().get_actions()),
                "axioms": len(translated_task.get_axioms()),
            } == configuration["translated_task"], expected["name"]


def test_high_level_bindings_expose_useful_docstrings() -> None:
    assert "Parse a PDDL domain" in (pypddl.Parser.__doc__ or "")
    assert "Return the parsed domain" in (pypddl.Parser.domain.__doc__ or "")
    assert "Parse a PDDL problem" in (pypddl.Parser.parse_task.__doc__ or "")
    assert "Translate and normalize" in (pypddl.translate_domain.__doc__ or "")
    assert "matching domain translation" in (pypddl.translate_task.__doc__ or "")
    assert "reparseable PDDL" in (pypddl.format_domain.__doc__ or "")
    assert "Factory for creating" in (pypddl.RepositoryFactory.__doc__ or "")
    assert "Create a repository" in (pypddl.RepositoryFactory.create.__doc__ or "")


def test_parser_views_keep_temporary_parser_alive() -> None:
    domain = pypddl.Parser(fixture_text("facade"), pypddl.ParserOptions()).domain()
    task = pypddl.Parser(fixture_text("facade"), pypddl.ParserOptions()).parse_task(fixture_text("facade", "task.pddl"))

    assert domain.get_name() == "facade"
    assert task.get_name() == "facade-task"
    assert task.get_domain().get_name() == "facade"


def test_child_view_getters_keep_parent_views_alive() -> None:
    domain = pypddl.Parser(fixture_text("facade"), pypddl.ParserOptions()).parse_task(fixture_text("facade", "task.pddl")).get_domain()

    assert domain.get_name() == "facade"


def test_optional_child_view_getters_keep_parent_views_alive() -> None:
    domain = pypddl.Parser(fixture_text("facade-format"), pypddl.ParserOptions()).domain()
    action = domain.get_actions()[0]
    action_precondition = action.get_precondition()
    action_effect = action.get_effect()
    del action
    assert action_precondition is not None
    assert action_effect is not None

    assert isinstance(action_precondition.get_variant(), pypddl.ConditionLiteral)
    assert isinstance(action_effect.get_variant(), pypddl.EffectAnd)

    task = pypddl.Parser(fixture_text("facade-format-numeric"), pypddl.ParserOptions()).parse_task(
        fixture_text("facade-format-numeric", "task.pddl")
    )
    task_goal = task.get_goal()
    task_metric = task.get_metric()
    del task
    assert task_goal is not None
    assert task_metric is not None

    assert isinstance(task_goal.get_variant(), pypddl.ConditionLiteral)
    assert task_metric.get_optimization_direction() == pypddl.OptimizationDirection.Minimize
    assert isinstance(task_metric.get_expression().get_variant(), pypddl.FunctionTerm)


def test_translation_views_keep_temporary_inputs_alive() -> None:
    original_domain = pypddl.translate_domain(
        pypddl.Parser(fixture_text("facade"), pypddl.ParserOptions()).domain(),
        pypddl.TranslatorOptions(),
    ).original_domain

    assert original_domain.get_name() == "facade"

    parser = pypddl.Parser(fixture_text("facade"), pypddl.ParserOptions())
    translator_options = pypddl.TranslatorOptions()
    domain_translation = pypddl.translate_domain(parser.domain(), translator_options)
    original_task = pypddl.translate_task(
        parser.parse_task(fixture_text("facade", "task.pddl")),
        domain_translation,
        translator_options,
    ).original_task
    del parser
    del domain_translation

    assert original_task.get_name() == "facade-task"
    assert original_task.get_domain().get_name() == "facade"


def test_translation_bindings_return_translated_views() -> None:
    parser = pypddl.Parser(fixture_text("facade"), pypddl.ParserOptions())
    domain = parser.domain()
    task = parser.parse_task(fixture_text("facade", "task.pddl"))

    options = pypddl.TranslatorOptions()
    assert options.compile_typing is True
    assert options.materialize_equality is True
    assert options.normalize_arithmetic_expressions is False
    options.normalize_arithmetic_expressions = True

    domain_translation = pypddl.translate_domain(domain, options)
    problem_translation = pypddl.translate_task(task, domain_translation, options)
    temporary_translated_task = pypddl.translate_task(task, domain_translation, options).translated_task

    assert domain_translation.original_domain.get_name() == "facade"
    assert domain_translation.translated_domain.get_actions()
    assert problem_translation.original_task.get_name() == "facade-task"
    assert problem_translation.translated_task.get_domain().get_name() == domain_translation.translated_domain.get_name()
    assert temporary_translated_task.get_domain().get_name() == domain_translation.translated_domain.get_name()


def test_arithmetic_normalization_binding_flattens_sorts_and_removes_units() -> None:
    parser_options = pypddl.ParserOptions()
    parser_options.add_action_costs = False
    parser = pypddl.Parser(
        """
        (define (domain arithmetic-normalization)
          (:requirements :strips :numeric-fluents)
          (:functions (f))
          (:action a
            :parameters ()
            :effect (assign (f) (+ 3 (+ 2 1) 0))))
        """,
        parser_options,
    )
    options = pypddl.TranslatorOptions()
    options.compile_typing = False
    options.materialize_equality = False
    options.normalize_arithmetic_expressions = True

    translated = pypddl.translate_domain(parser.domain(), options).translated_domain

    assert "(assign (f) (+ 1 2 3))" in pypddl.format_domain(translated)


def test_repository_view_keeps_temporary_repository_alive() -> None:
    view, inserted = pypddl.RepositoryFactory().create().insert(pypddl.TypeData("temporary-type"))
    gc.collect()

    assert inserted is True
    assert view.get_name() == "temporary-type"


def test_insert_returns_flag_and_retains_repository_on_extracted_view() -> None:
    repository = pypddl.RepositoryFactory().create()
    assert not hasattr(repository, "get_or_create")
    before = sys.getrefcount(repository)
    result = repository.insert(pypddl.TypeData("retained-type"))
    assert isinstance(result, tuple)
    view, inserted = result
    assert inserted is True
    duplicate, duplicate_inserted = repository.insert(pypddl.TypeData("retained-type"))
    assert duplicate_inserted is False
    assert duplicate == view
    del duplicate, result
    assert sys.getrefcount(repository) > before
    del repository
    gc.collect()
    assert view.get_name() == "retained-type"


def test_semantic_exceptions_have_expected_hierarchy() -> None:
    semantic_error_names = [
        "ParseError",
        "MissingDomainError",
        "MismatchedDomainError",
        "UnsupportedRequirementError",
        "MissingRequirementError",
        "UnusedRequirementError",
        "UndefinedTypeError",
        "UndefinedPredicateError",
        "UndefinedObjectError",
        "UndefinedVariableError",
        "UndefinedFunctionError",
        "ArityMismatchError",
        "TypeMismatchError",
        "InvalidMetricError",
        "InvalidNumericConstraintError",
        "InvalidNumericEffectError",
        "InvalidProbabilisticEffectError",
        "InvalidEqualityError",
    ]
    for name in semantic_error_names:
        assert issubclass(getattr(pypddl, name), pypddl.SemanticError)

    duplicate_error_names = [
        "DuplicateTypeError",
        "DuplicatePredicateError",
        "DuplicateObjectError",
        "DuplicateVariableError",
        "DuplicateFunctionError",
    ]
    for name in duplicate_error_names:
        error = getattr(pypddl, name)
        assert issubclass(error, pypddl.DuplicateDefinitionError)
        assert issubclass(error, pypddl.SemanticError)


def make_literal(repository: pypddl.Repository, predicate: pypddl.Predicate, terms: Iterable[pypddl.Term], positive: bool = True) -> pypddl.Literal:
    atom, _ = repository.insert(pypddl.AtomData(predicate, list(terms)))
    return repository.insert(pypddl.LiteralData(atom, positive))[0]


def make_condition(repository: pypddl.Repository, predicate: pypddl.Predicate, terms: Iterable[pypddl.Term], positive: bool = True) -> pypddl.Condition:
    condition_literal, _ = repository.insert(pypddl.ConditionLiteralData(make_literal(repository, predicate, terms, positive)))
    return repository.insert(pypddl.ConditionData(variant=condition_literal))[0]


def make_effect(repository: pypddl.Repository, literal: pypddl.Literal) -> pypddl.Effect:
    effect_literal, _ = repository.insert(pypddl.EffectLiteralData(literal))
    return repository.insert(pypddl.EffectData(variant=effect_literal))[0]


def make_number_expression(repository: pypddl.Repository, value: float) -> pypddl.FunctionExpression:
    number, _ = repository.insert(pypddl.FunctionExpressionNumberData(value))
    return repository.insert(pypddl.FunctionExpressionData(variant=number))[0]


def test_builders_expose_defaulted_mutable_fields() -> None:
    repository = pypddl.RepositoryFactory().create()

    object_type, _ = repository.insert(pypddl.TypeData("object"))
    variable, _ = repository.insert(pypddl.VariableData("?x"))
    parameter, _ = repository.insert(pypddl.ParameterData(variable))
    predicate, _ = repository.insert(pypddl.PredicateData("p"))
    term, _ = repository.insert(pypddl.TermData(variant=variable))
    literal = make_literal(repository, predicate, [term])
    condition = make_condition(repository, predicate, [term])
    effect = make_effect(repository, literal)

    parameter_builder = pypddl.ParameterData(variable)
    assert parameter_builder.variable == variable.get_index()
    assert parameter_builder.types == []
    parameter_builder.types = [object_type.get_index()]
    assert parameter_builder.types == [object_type.get_index()]

    action_builder = pypddl.ActionData("a")
    assert action_builder.name == "a"
    assert action_builder.parameters == []
    assert action_builder.precondition is None
    assert action_builder.effect is None

    explicit_action_builder = pypddl.ActionData("generated-action", "original-action", [parameter], 3, condition, effect)
    assert explicit_action_builder.name == "generated-action"
    assert explicit_action_builder.original_name == "original-action"
    assert explicit_action_builder.parameters == [parameter.get_index()]
    assert explicit_action_builder.original_arity == 3
    assert explicit_action_builder.precondition == condition.get_index()
    assert explicit_action_builder.effect == effect.get_index()

    action_builder.name = "mutated-action"
    action_builder.parameters = [parameter.get_index()]
    action_builder.original_arity = len(action_builder.parameters)
    action_builder.precondition = condition.get_index()
    action_builder.effect = effect.get_index()
    action, _ = repository.insert(action_builder)

    domain_builder = pypddl.DomainData("d")
    assert domain_builder.requirements == []
    assert domain_builder.types == []
    assert domain_builder.constants == []
    assert domain_builder.predicates == []
    assert domain_builder.functions == []
    assert domain_builder.actions == []
    assert domain_builder.axioms == []
    domain_builder.name = "mutated-domain"
    domain_builder.types = [object_type.get_index()]
    domain_builder.predicates = [predicate.get_index()]
    domain_builder.actions = [action.get_index()]
    domain, _ = repository.insert(domain_builder)

    task_builder = pypddl.TaskData("t", domain)
    assert task_builder.name == "t"
    assert task_builder.domain == domain.get_index()
    assert task_builder.requirements == []
    assert task_builder.objects == []
    assert task_builder.initial_literals == []
    assert task_builder.initial_function_values == []
    assert task_builder.goal is None
    assert task_builder.metric is None
    assert task_builder.predicates == []
    assert task_builder.axioms == []
    task_builder.name = "mutated-task"
    task_builder.goal = condition.get_index()
    task, _ = repository.insert(task_builder)

    assert action.get_name() == "mutated-action"
    assert len(action.get_parameters()) == 1
    assert action.get_arity() == 1
    assert action.get_original_arity() == 1
    assert action.get_precondition() is not None
    assert action.get_effect() is not None
    assert domain.get_name() == "mutated-domain"
    assert len(domain.get_types()) == 1
    assert len(domain.get_predicates()) == 1
    assert len(domain.get_actions()) == 1
    assert task.get_name() == "mutated-task"
    assert task.get_domain() == domain
    assert task.get_goal() is not None


def test_repository_constructs_numeric_function_task_bits() -> None:
    repository = pypddl.RepositoryFactory().create()
    numeric, _ = repository.insert(pypddl.RequirementData(pypddl.RequirementKind.NumericFluents))
    object_type, _ = repository.insert(pypddl.TypeData("object"))
    number_type, _ = repository.insert(pypddl.TypeData("number"))
    variable, _ = repository.insert(pypddl.VariableData("?x"))
    parameter, _ = repository.insert(pypddl.ParameterData(variable, [object_type]))
    location, _ = repository.insert(pypddl.ObjectData("l1", [object_type]))
    fluent, _ = repository.insert(pypddl.FunctionSkeletonData("fuel", [parameter], number_type))
    term, _ = repository.insert(pypddl.TermData(variant=variable))
    function_term, _ = repository.insert(pypddl.FunctionTermData(fluent, [term]))
    zero = make_number_expression(repository, 0.0)
    one = make_number_expression(repository, 1.0)
    numeric_node, _ = repository.insert(pypddl.ConditionNumericConstraintData(pypddl.BinaryComparator.Ge, zero, zero))
    condition, _ = repository.insert(pypddl.ConditionData(numeric_node))
    numeric_effect_node, _ = repository.insert(pypddl.EffectNumericData(pypddl.NumericEffectOperator.Assign, function_term, one))
    effect, _ = repository.insert(pypddl.EffectData(numeric_effect_node))
    action, _ = repository.insert(pypddl.ActionData("refuel", [parameter], condition, effect))
    initial_value, _ = repository.insert(pypddl.InitialFunctionValueData(function_term, zero))
    metric, _ = repository.insert(pypddl.MetricData(pypddl.OptimizationDirection.Minimize, one))
    domain, _ = repository.insert(pypddl.DomainData(
            "numeric-programmatic",
            requirements=[numeric],
            types=[object_type, number_type],
            functions=[fluent],
            actions=[action],
        ),
    )
    task, _ = repository.insert(pypddl.TaskData(
            "numeric-programmatic-task",
            domain,
            objects=[location],
            initial_function_values=[initial_value],
            goal=condition,
            metric=metric,
        ),
    )

    assert domain.get_functions()[0].get_name() == "fuel"
    assert domain.get_functions()[0].get_arity() == 1
    assert len(domain.get_functions()[0].get_parameters()) == 1
    assert domain.get_functions()[0].get_type().get_name() == "number"
    assert task.get_initial_function_values()[0].get_function().get_function().get_name() == "fuel"
    assert len(task.get_initial_function_values()[0].get_function().get_terms()) == 1
    numeric_constraint = condition.get_variant()
    assert isinstance(numeric_constraint, pypddl.ConditionNumericConstraint)
    assert numeric_constraint.get_comparator() == pypddl.BinaryComparator.Ge
    left = numeric_constraint.get_left().get_variant()
    assert isinstance(left, pypddl.FunctionExpressionNumber)
    assert left.get_value() == 0.0
    right = numeric_constraint.get_right().get_variant()
    assert isinstance(right, pypddl.FunctionExpressionNumber)
    assert right.get_value() == 0.0
    numeric_effect = effect.get_variant()
    assert isinstance(numeric_effect, pypddl.EffectNumeric)
    assert numeric_effect.get_operator() == pypddl.NumericEffectOperator.Assign
    numeric_effect_value = numeric_effect.get_expression().get_variant()
    assert isinstance(numeric_effect_value, pypddl.FunctionExpressionNumber)
    assert numeric_effect_value.get_value() == 1.0
    assert task.get_goal() is not None
    metric = task.get_metric()
    assert metric is not None
    assert metric.get_optimization_direction() == pypddl.OptimizationDirection.Minimize
    metric_value = metric.get_expression().get_variant()
    assert isinstance(metric_value, pypddl.FunctionExpressionNumber)
    assert metric_value.get_value() == 1.0


def test_views_expose_typed_indices() -> None:
    parser = pypddl.Parser(fixture_text("facade-format"), pypddl.ParserOptions())
    domain = parser.domain()

    domain_index = domain.get_index()
    next_domain_index = pypddl.DomainIndex(1)

    assert isinstance(domain_index, pypddl.DomainIndex)
    assert int(domain_index) == 0
    assert operator.index(domain_index) == 0
    assert str(domain_index) == repr(domain_index) == "DomainIndex(0)"
    assert hash(domain_index) == 0
    assert domain_index == domain.get_index()
    assert domain_index != next_domain_index
    assert domain_index < next_domain_index
    assert domain_index <= domain.get_index()
    assert next_domain_index > domain_index
    assert next_domain_index >= domain_index

    predicate_index = domain.get_predicates()[0].get_index()
    assert isinstance(predicate_index, pypddl.PredicateIndex)
    assert int(predicate_index) == 0
    assert str(predicate_index) == repr(predicate_index) == "PredicateIndex(0)"


def test_data_compare_by_identifying_members_and_are_unhashable() -> None:
    strips = pypddl.RequirementData(pypddl.RequirementKind.Strips)
    same_strips = pypddl.RequirementData(pypddl.RequirementKind.Strips)
    typing = pypddl.RequirementData(pypddl.RequirementKind.Typing)

    assert strips == same_strips
    assert strips != typing
    assert strips < typing
    assert strips <= same_strips
    assert typing > strips
    assert typing >= strips
    assert sorted([typing, strips]) == [strips, typing]

    try:
        hash(strips)
    except TypeError:
        pass
    else:
        raise AssertionError("mutable Data bindings must be unhashable")


def test_views_are_hashable_and_compare_by_identity() -> None:
    parser = pypddl.Parser(fixture_text("facade-format"), pypddl.ParserOptions())
    domain = parser.domain()
    same_domain = parser.domain()
    equivalent_domain = pypddl.Parser(fixture_text("facade-format"), pypddl.ParserOptions()).domain()

    assert domain == same_domain
    assert hash(domain) == hash(same_domain)
    assert domain == equivalent_domain
    assert hash(domain) == hash(equivalent_domain)
    assert len({domain, same_domain, equivalent_domain}) == 1
    assert str(domain) == repr(domain)
    assert "facade-format" in str(domain)

    first_predicate = domain.get_predicates()[0]
    same_predicate = same_domain.get_predicates()[0]
    second_predicate = domain.get_predicates()[1]

    assert first_predicate == same_predicate
    assert hash(first_predicate) == hash(same_predicate)
    assert first_predicate != second_predicate
    assert first_predicate < second_predicate
    assert first_predicate <= same_predicate
    assert second_predicate > first_predicate
    assert second_predicate >= first_predicate
    assert len({first_predicate, same_predicate, second_predicate}) == 2
    assert str(first_predicate) == repr(first_predicate)
    assert "ready" in str(first_predicate)


def test_translate_task_from_different_domain_translation_uses_typed_exception() -> None:
    first_translation = pypddl.translate_domain(
        pypddl.Parser(fixture_text("facade"), pypddl.ParserOptions()).domain(),
        pypddl.TranslatorOptions(),
    )
    second_parser = pypddl.Parser(fixture_text("repo-mismatch"), pypddl.ParserOptions())
    second_task = second_parser.parse_task(fixture_text("repo-mismatch", "task.pddl"))

    try:
        pypddl.translate_task(second_task, first_translation, pypddl.TranslatorOptions())
    except pypddl.MismatchedDomainError:
        pass
    else:
        raise AssertionError("expected mismatched domain diagnostic")


def test_translate_task_task_only_equality_uses_typed_exception() -> None:
    parser = pypddl.Parser(fixture_text("task-only-equality"), pypddl.ParserOptions())
    options = pypddl.TranslatorOptions()
    options.materialize_equality = True
    domain_translation = pypddl.translate_domain(parser.domain(), options)
    task = parser.parse_task(fixture_text("task-only-equality", "task.pddl"))

    try:
        pypddl.translate_task(task, domain_translation, options)
    except pypddl.InvalidEqualityError as error:
        assert "equality predicate" in str(error)
    else:
        raise AssertionError("expected invalid equality diagnostic")


def test_recursive_variant_views_are_inspectable() -> None:
    options = pypddl.ParserOptions()
    options.add_action_costs = False
    parser = pypddl.Parser(fixture_text("when-or-effect"), options)
    action = parser.domain().get_actions()[0]

    action_effect = action.get_effect()
    assert action_effect is not None
    effect = action_effect.get_variant()
    assert isinstance(effect, pypddl.EffectWhen)
    or_condition = effect.get_condition().get_variant()
    assert isinstance(or_condition, pypddl.ConditionOr)
    assert len(or_condition.get_conditions()) == 2
    assert isinstance(effect.get_effect().get_variant(), pypddl.EffectAnd)


def test_numeric_expression_variant_views_are_inspectable() -> None:
    repository = pypddl.RepositoryFactory().create()
    number = make_number_expression(repository, 2.0)

    variant = number.get_variant()
    assert isinstance(variant, pypddl.FunctionExpressionNumber)
    assert variant.get_value() == 2.0


def test_multi_args_keep_repository_alive() -> None:
    def make_args() -> list[pypddl.FunctionExpression]:
        repository = pypddl.RepositoryFactory().create()
        operands = [make_number_expression(repository, value) for value in (1.0, 2.0, 3.0)]
        multi, _ = repository.insert(
            pypddl.MultiFunctionExpressionData(
                pypddl.MultiArithmeticOperator.Add,
                operands,
            )
        )
        return cast(list[pypddl.FunctionExpression], multi.get_args())

    args = make_args()
    gc.collect()

    assert args[2].get_variant().get_value() == 3.0


def test_repository_exposes_recursive_constructors_and_accessors() -> None:
    repository = pypddl.RepositoryFactory().create()
    object_type, _ = repository.insert(pypddl.TypeData("object"))
    number_type, _ = repository.insert(pypddl.TypeData("number"))
    variable, _ = repository.insert(pypddl.VariableData("?x"))
    parameter, _ = repository.insert(pypddl.ParameterData(variable, [object_type]))
    predicate, _ = repository.insert(pypddl.PredicateData("p", [parameter]))
    term, _ = repository.insert(pypddl.TermData(variant=variable))
    term_variant = term.get_variant()
    assert isinstance(term_variant, pypddl.Variable)
    assert term_variant.get_name() == "?x"
    literal = make_literal(repository, predicate, [term])
    base_condition = make_condition(repository, predicate, [term])
    condition_not_node, _ = repository.insert(pypddl.ConditionNotData(base_condition))
    condition_not, _ = repository.insert(pypddl.ConditionData(condition_not_node))
    condition_or_node, _ = repository.insert(pypddl.ConditionOrData([base_condition, condition_not]))
    condition_or, _ = repository.insert(pypddl.ConditionData(condition_or_node))
    condition_and_node, _ = repository.insert(pypddl.ConditionAndData([base_condition, condition_not]))
    condition_and, _ = repository.insert(pypddl.ConditionData(condition_and_node))
    condition_imply_node, _ = repository.insert(pypddl.ConditionImplyData(base_condition, condition_or))
    condition_imply, _ = repository.insert(pypddl.ConditionData(condition_imply_node))
    condition_exists_node, _ = repository.insert(pypddl.ConditionExistsData([parameter], condition_imply))
    condition_exists, _ = repository.insert(pypddl.ConditionData(condition_exists_node))
    condition_forall_node, _ = repository.insert(pypddl.ConditionForallData([parameter], condition_exists))
    condition_forall, _ = repository.insert(pypddl.ConditionData(condition_forall_node))

    number = make_number_expression(repository, 1.0)
    unary_node, _ = repository.insert(pypddl.UnaryFunctionExpressionData(pypddl.UnaryArithmeticOperator.Sub, number))
    unary, _ = repository.insert(pypddl.FunctionExpressionData(unary_node))
    binary_node, _ = repository.insert(pypddl.BinaryFunctionExpressionData(pypddl.BinaryArithmeticOperator.Add, number, unary))
    binary, _ = repository.insert(pypddl.FunctionExpressionData(binary_node))
    multi_node, _ = repository.insert(pypddl.MultiFunctionExpressionData(pypddl.MultiArithmeticOperator.Add, [number, binary]))
    multi, _ = repository.insert(pypddl.FunctionExpressionData(multi_node))

    fluent, _ = repository.insert(pypddl.FunctionSkeletonData("f", [parameter], number_type))
    function_term, _ = repository.insert(pypddl.FunctionTermData(fluent, [term]))
    numeric_effect_node, _ = repository.insert(pypddl.EffectNumericData(pypddl.NumericEffectOperator.Assign, function_term, multi))
    numeric_effect, _ = repository.insert(pypddl.EffectData(numeric_effect_node))
    literal_effect = make_effect(repository, literal)
    effect_when_node, _ = repository.insert(pypddl.EffectWhenData(condition_forall, literal_effect))
    effect_when, _ = repository.insert(pypddl.EffectData(effect_when_node))
    effect_forall_node, _ = repository.insert(pypddl.EffectForallData([parameter], effect_when))
    effect_forall, _ = repository.insert(pypddl.EffectData(effect_forall_node))
    effect_one_of_node, _ = repository.insert(pypddl.EffectOneOfData([literal_effect, numeric_effect]))
    effect_one_of, _ = repository.insert(pypddl.EffectData(effect_one_of_node))
    alt1, _ = repository.insert(pypddl.EffectProbabilisticAlternativeData(0.4, literal_effect))
    alt2, _ = repository.insert(pypddl.EffectProbabilisticAlternativeData(0.6, effect_one_of))
    effect_probabilistic_node, _ = repository.insert(pypddl.EffectProbabilisticData([alt1, alt2]))
    effect_probabilistic, _ = repository.insert(pypddl.EffectData(effect_probabilistic_node))
    axiom, _ = repository.insert(pypddl.AxiomData([parameter], literal, condition_forall))
    action, _ = repository.insert(pypddl.ActionData("a", [parameter], condition_forall, effect_probabilistic))
    domain, _ = repository.insert(pypddl.DomainData("full-builder", types=[object_type, number_type], predicates=[predicate], functions=[fluent], actions=[action], axioms=[axiom]))
    task, _ = repository.insert(pypddl.TaskData("full-builder-task", domain, goal=condition_forall, axioms=[axiom]))

    condition_forall_variant = condition_forall.get_variant()
    assert isinstance(condition_forall_variant, pypddl.ConditionForall)
    assert len(condition_forall_variant.get_parameters()) == 1
    condition_imply_variant = condition_imply.get_variant()
    assert isinstance(condition_imply_variant, pypddl.ConditionImply)
    assert condition_imply_variant.get_left() == base_condition
    assert condition_imply_variant.get_right() == condition_or
    condition_exists_variant = condition_exists.get_variant()
    assert isinstance(condition_exists_variant, pypddl.ConditionExists)
    assert len(condition_exists_variant.get_parameters()) == 1
    assert isinstance(condition_or.get_variant(), pypddl.ConditionOr)
    assert isinstance(condition_and.get_variant(), pypddl.ConditionAnd)
    effect_forall_variant = effect_forall.get_variant()
    assert isinstance(effect_forall_variant, pypddl.EffectForall)
    assert len(effect_forall_variant.get_parameters()) == 1
    effect_one_of_variant = effect_one_of.get_variant()
    assert isinstance(effect_one_of_variant, pypddl.EffectOneOf)
    assert len(effect_one_of_variant.get_effects()) == 2
    effect_probabilistic_variant = effect_probabilistic.get_variant()
    assert isinstance(effect_probabilistic_variant, pypddl.EffectProbabilistic)
    assert len(effect_probabilistic_variant.get_alternatives()) == 2
    numeric_effect_variant = numeric_effect.get_variant()
    assert isinstance(numeric_effect_variant, pypddl.EffectNumeric)
    assert numeric_effect_variant.get_operator() == pypddl.NumericEffectOperator.Assign
    assert len(numeric_effect_variant.get_function().get_terms()) == 1
    unary_variant = unary.get_variant()
    assert isinstance(unary_variant, pypddl.UnaryFunctionExpression)
    assert unary_variant.get_operator() == pypddl.UnaryArithmeticOperator.Sub
    binary_variant = binary.get_variant()
    assert isinstance(binary_variant, pypddl.BinaryFunctionExpression)
    assert binary_variant.get_operator() == pypddl.BinaryArithmeticOperator.Add
    multi_variant = multi.get_variant()
    assert isinstance(multi_variant, pypddl.MultiFunctionExpression)
    assert multi_variant.get_operator() == pypddl.MultiArithmeticOperator.Add
    multi_operands = multi_variant.get_args()
    assert len(multi_operands) == 2
    assert number in multi_operands
    assert binary in multi_operands
    assert effect_probabilistic_variant.get_alternatives()[0].get_probability() == 0.4
    assert domain.get_axioms()[0].get_arity() == 1
    assert domain.get_axioms()[0].get_original_arity() == 1
    assert len(domain.get_axioms()[0].get_parameters()) == 1
    assert isinstance(domain.get_axioms()[0].get_condition().get_variant(), pypddl.ConditionForall)
    task_goal = task.get_goal()
    assert task_goal is not None
    assert isinstance(task_goal.get_variant(), pypddl.ConditionForall)

if __name__ == "__main__":
    from direct_runner import run_tests

    run_tests(globals())
