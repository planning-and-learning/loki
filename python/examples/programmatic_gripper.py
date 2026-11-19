# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later
"""Build a gripper domain and two tasks programmatically with pypddl's builders.

A builder is a record being built: its fields take keys, views or other builders, so a whole domain is written as
one nested value and interned once (Repository.insert), its children first. Equal records intern to one, so a
builder used in several places (a parameter, a type) becomes one record.
"""

from collections.abc import Sequence

from pypddl import formalism as pypddl

Term = pypddl.VariableBuilder | pypddl.ObjectBuilder
Condition = pypddl.ConditionLiteralBuilder | pypddl.ConditionAndBuilder
Effect = pypddl.EffectLiteralBuilder | pypddl.EffectAndBuilder


def type_(name: str, *bases: pypddl.TypeBuilder) -> pypddl.TypeBuilder:
    """The type name, a subtype of bases."""
    return pypddl.TypeBuilder(name=name, bases=list(bases))


def object_(name: str, type_of: pypddl.TypeBuilder) -> pypddl.ObjectBuilder:
    """The object name, of a type."""
    return pypddl.ObjectBuilder(name=name, types=[type_of])


def parameter(variable: pypddl.VariableBuilder, type_of: pypddl.TypeBuilder) -> pypddl.ParameterBuilder:
    """The parameter of a variable, of a type."""
    return pypddl.ParameterBuilder(variable=variable, types=[type_of])


def predicate(name: str, *parameters: pypddl.ParameterBuilder) -> pypddl.PredicateBuilder:
    """The predicate name over parameters."""
    return pypddl.PredicateBuilder(name=name, parameters=list(parameters))


def literal(predicate_of: pypddl.PredicateBuilder, *terms: Term, positive: bool = True) -> pypddl.LiteralBuilder:
    """The literal of a predicate over terms."""
    return pypddl.LiteralBuilder(atom=pypddl.AtomBuilder(predicate=predicate_of, terms=list(terms)), polarity=positive)


def condition(predicate_of: pypddl.PredicateBuilder, *terms: Term, positive: bool = True) -> pypddl.ConditionLiteralBuilder:
    """The condition that holds the literal of a predicate over terms."""
    return pypddl.ConditionLiteralBuilder(literal=literal(predicate_of, *terms, positive=positive))


def effect(predicate_of: pypddl.PredicateBuilder, *terms: Term, positive: bool = True) -> pypddl.EffectLiteralBuilder:
    """The effect that sets the literal of a predicate over terms."""
    return pypddl.EffectLiteralBuilder(literal=literal(predicate_of, *terms, positive=positive))


def conjunction(*conditions: Condition) -> pypddl.ConditionAndBuilder:
    """The conjunction of conditions."""
    return pypddl.ConditionAndBuilder(conditions=list(conditions))


def effects(*parts: Effect) -> pypddl.EffectAndBuilder:
    """The conjunction of effects."""
    return pypddl.EffectAndBuilder(effects=list(parts))


def action(
    name: str, parameters: Sequence[pypddl.ParameterBuilder], precondition: Condition, effect_of: Effect
) -> pypddl.ActionBuilder:
    """The action name over parameters, as written (its original name and arity its own)."""
    return pypddl.ActionBuilder(
        name=name,
        original_name=name,
        parameters=list(parameters),
        original_arity=len(parameters),
        precondition=precondition,
        effect=effect_of,
    )


def requirement(kind: pypddl.RequirementKind) -> pypddl.RequirementBuilder:
    """The requirement of a kind."""
    return pypddl.RequirementBuilder(kind=kind)


def domain_(
    name: str,
    requirements: Sequence[pypddl.RequirementBuilder],
    types: Sequence[pypddl.TypeBuilder],
    predicates: Sequence[pypddl.PredicateBuilder],
    actions: Sequence[pypddl.ActionBuilder],
) -> pypddl.DomainBuilder:
    """The domain name with its parts."""
    return pypddl.DomainBuilder(
        name=name, requirements=list(requirements), types=list(types), predicates=list(predicates), actions=list(actions)
    )


def task(
    name: str,
    domain: pypddl.DomainBuilder,
    objects: Sequence[pypddl.ObjectBuilder],
    initial_literals: Sequence[pypddl.LiteralBuilder],
    goal: Condition,
) -> pypddl.TaskBuilder:
    """The task name of a domain."""
    return pypddl.TaskBuilder(
        name=name, domain=domain, objects=list(objects), initial_literals=list(initial_literals), goal=goal
    )


def build_gripper() -> tuple[pypddl.Repository, pypddl.Domain, pypddl.Task, pypddl.Task]:
    """Build the gripper domain and two of its tasks."""
    object_t = type_("object")
    room_t, ball_t, gripper_t = type_("room", object_t), type_("ball", object_t), type_("gripper", object_t)

    x_var, y_var, b_var, g_var = (pypddl.VariableBuilder(name=name) for name in ("?x", "?y", "?b", "?g"))
    x, y = parameter(x_var, room_t), parameter(y_var, room_t)
    b, g = parameter(b_var, ball_t), parameter(g_var, gripper_t)

    at_robby = predicate("at-robby", x)
    at = predicate("at", b, x)
    free = predicate("free", g)
    carry = predicate("carry", b, g)

    move = action(
        "move",
        [x, y],
        condition(at_robby, x_var),
        effects(effect(at_robby, x_var, positive=False), effect(at_robby, y_var)),
    )
    pick = action(
        "pick",
        [b, x, g],
        conjunction(condition(at, b_var, x_var), condition(at_robby, x_var), condition(free, g_var)),
        effects(effect(at, b_var, x_var, positive=False), effect(free, g_var, positive=False), effect(carry, b_var, g_var)),
    )
    drop = action(
        "drop",
        [b, x, g],
        conjunction(condition(carry, b_var, g_var), condition(at_robby, x_var)),
        effects(effect(carry, b_var, g_var, positive=False), effect(free, g_var), effect(at, b_var, x_var)),
    )

    domain = domain_(
        "gripper",
        requirements=[requirement(pypddl.RequirementKind.Strips), requirement(pypddl.RequirementKind.Typing)],
        types=[object_t, room_t, ball_t, gripper_t],
        predicates=[at_robby, at, free, carry],
        actions=[move, pick, drop],
    )

    rooma, roomb = object_("rooma", room_t), object_("roomb", room_t)
    left, right = object_("left", gripper_t), object_("right", gripper_t)
    ball1, ball2 = object_("ball1", ball_t), object_("ball2", ball_t)

    one_ball = task(
        "gripper-1-ball",
        domain,
        objects=[rooma, roomb, left, right, ball1],
        initial_literals=[literal(at_robby, rooma), literal(free, left), literal(free, right), literal(at, ball1, rooma)],
        goal=conjunction(condition(at, ball1, roomb)),
    )
    two_ball = task(
        "gripper-2-ball",
        domain,
        objects=[rooma, roomb, left, right, ball1, ball2],
        initial_literals=[
            literal(at_robby, rooma),
            literal(free, left),
            literal(free, right),
            literal(at, ball1, rooma),
            literal(at, ball2, rooma),
        ],
        goal=conjunction(condition(at, ball1, roomb), condition(at, ball2, roomb)),
    )

    # Interned once each, children first: the tasks' domain is the domain's record.
    repository = pypddl.Repository()
    interned_domain, _ = repository.insert(domain)
    interned_one_ball, _ = repository.insert(one_ball)
    interned_two_ball, _ = repository.insert(two_ball)
    return repository, interned_domain, interned_one_ball, interned_two_ball


if __name__ == "__main__":
    _, domain, one_ball, two_ball = build_gripper()
    print(
        f"domain: {domain.get_name()}, actions={len(domain.get_actions())}, predicates={len(domain.get_predicates())}"
    )
    print(
        f"task: {one_ball.get_name()}, objects={len(one_ball.get_objects())},",
        f"init={len(one_ball.get_initial_literals())}",
    )
    print(
        f"task: {two_ball.get_name()}, objects={len(two_ball.get_objects())},",
        f"init={len(two_ball.get_initial_literals())}",
    )
