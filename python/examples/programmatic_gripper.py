# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later
"""Build a gripper domain and two tasks programmatically with pypddl."""

from collections.abc import Sequence

from pypddl import formalism as pypddl


def make_literal(
    repository: pypddl.Repository,
    predicate: pypddl.Predicate,
    terms: Sequence[pypddl.Variable | pypddl.Object],
    *,
    positive: bool = True,
) -> pypddl.Literal:
    """Intern the literal of predicate over terms."""
    atom, _ = repository.insert(pypddl.AtomData(predicate.index(), [each.index() for each in terms]))
    return repository.insert(pypddl.LiteralData(atom.index(), positive))[0]


def make_condition(
    repository: pypddl.Repository,
    predicate: pypddl.Predicate,
    terms: Sequence[pypddl.Variable | pypddl.Object],
    *,
    positive: bool = True,
) -> pypddl.ConditionLiteral:
    """Intern the condition that holds the literal of predicate over terms."""
    literal = make_literal(repository, predicate, terms, positive=positive)
    condition_literal, _ = repository.insert(pypddl.ConditionLiteralData(literal.index()))
    return condition_literal


def make_effect(repository: pypddl.Repository, literal: pypddl.Literal) -> pypddl.EffectLiteral:
    """Intern the effect that sets literal."""
    effect_literal, _ = repository.insert(pypddl.EffectLiteralData(literal.index()))
    return effect_literal


def make_condition_and(
    repository: pypddl.Repository,
    conditions: Sequence[pypddl.ConditionLiteral],
) -> pypddl.ConditionAnd:
    """Intern the conjunction of conditions."""
    conjunction, _ = repository.insert(pypddl.ConditionAndData([each.index() for each in conditions]))
    return conjunction


def make_effect_and(
    repository: pypddl.Repository,
    effects: Sequence[pypddl.EffectLiteral],
) -> pypddl.EffectAnd:
    """Intern the conjunction of effects."""
    conjunction, _ = repository.insert(pypddl.EffectAndData([each.index() for each in effects]))
    return conjunction


def build_gripper() -> tuple[pypddl.Repository, pypddl.Domain, pypddl.Task, pypddl.Task]:
    """Build the gripper domain and two of its tasks."""
    repository = pypddl.Repository()
    strips, _ = repository.insert(pypddl.RequirementData(pypddl.RequirementKind.Strips))
    typing, _ = repository.insert(pypddl.RequirementData(pypddl.RequirementKind.Typing))

    object_t, _ = repository.insert(pypddl.TypeData("object", []))
    room_t, _ = repository.insert(pypddl.TypeData("room", [object_t.index()]))
    ball_t, _ = repository.insert(pypddl.TypeData("ball", [object_t.index()]))
    gripper_t, _ = repository.insert(pypddl.TypeData("gripper", [object_t.index()]))

    x, _ = repository.insert(
        pypddl.ParameterData(repository.insert(pypddl.VariableData("?x"))[0].index(), [room_t.index()])
    )
    y, _ = repository.insert(
        pypddl.ParameterData(repository.insert(pypddl.VariableData("?y"))[0].index(), [room_t.index()])
    )
    b, _ = repository.insert(
        pypddl.ParameterData(repository.insert(pypddl.VariableData("?b"))[0].index(), [ball_t.index()])
    )
    g, _ = repository.insert(
        pypddl.ParameterData(repository.insert(pypddl.VariableData("?g"))[0].index(), [gripper_t.index()])
    )

    at_robby, _ = repository.insert(pypddl.PredicateData("at-robby", [x.index()]))
    at, _ = repository.insert(pypddl.PredicateData("at", [b.index(), x.index()]))
    free, _ = repository.insert(pypddl.PredicateData("free", [g.index()]))
    carry, _ = repository.insert(pypddl.PredicateData("carry", [b.index(), g.index()]))

    x_term = x.get_variable()
    y_term = y.get_variable()
    b_term = b.get_variable()
    g_term = g.get_variable()

    move_pre = make_condition(repository, at_robby, [x_term])
    move_eff = make_effect_and(
        repository,
        [
            make_effect(repository, make_literal(repository, at_robby, [x_term], positive=False)),
            make_effect(repository, make_literal(repository, at_robby, [y_term])),
        ],
    )
    move, _ = repository.insert(
        pypddl.ActionData("move", "move", [x.index(), y.index()], 2, move_pre.index(), move_eff.index())
    )

    pick_pre = make_condition_and(
        repository,
        [
            make_condition(repository, at, [b_term, x_term]),
            make_condition(repository, at_robby, [x_term]),
            make_condition(repository, free, [g_term]),
        ],
    )
    pick_eff = make_effect_and(
        repository,
        [
            make_effect(repository, make_literal(repository, at, [b_term, x_term], positive=False)),
            make_effect(repository, make_literal(repository, free, [g_term], positive=False)),
            make_effect(repository, make_literal(repository, carry, [b_term, g_term])),
        ],
    )
    pick, _ = repository.insert(
        pypddl.ActionData("pick", "pick", [b.index(), x.index(), g.index()], 3, pick_pre.index(), pick_eff.index())
    )

    drop_pre = make_condition_and(
        repository,
        [
            make_condition(repository, carry, [b_term, g_term]),
            make_condition(repository, at_robby, [x_term]),
        ],
    )
    drop_eff = make_effect_and(
        repository,
        [
            make_effect(repository, make_literal(repository, carry, [b_term, g_term], positive=False)),
            make_effect(repository, make_literal(repository, free, [g_term])),
            make_effect(repository, make_literal(repository, at, [b_term, x_term])),
        ],
    )
    drop, _ = repository.insert(
        pypddl.ActionData("drop", "drop", [b.index(), x.index(), g.index()], 3, drop_pre.index(), drop_eff.index())
    )

    domain, _ = repository.insert(
        pypddl.DomainData(
            "gripper",
            requirements=[strips.index(), typing.index()],
            types=[object_t.index(), room_t.index(), ball_t.index(), gripper_t.index()],
            constants=[],
            predicates=[at_robby.index(), at.index(), free.index(), carry.index()],
            functions=[],
            actions=[move.index(), pick.index(), drop.index()],
            axioms=[],
        ),
    )

    def task(name: str, balls: Sequence[str]) -> pypddl.Task:
        rooma, _ = repository.insert(pypddl.ObjectData("rooma", [room_t.index()]))
        roomb, _ = repository.insert(pypddl.ObjectData("roomb", [room_t.index()]))
        left, _ = repository.insert(pypddl.ObjectData("left", [gripper_t.index()]))
        right, _ = repository.insert(pypddl.ObjectData("right", [gripper_t.index()]))
        ball_objects = [repository.insert(pypddl.ObjectData(ball, [ball_t.index()]))[0] for ball in balls]

        rooma_term = rooma
        roomb_term = roomb
        left_term = left
        right_term = right

        initial_literals = [
            make_literal(repository, at_robby, [rooma_term]),
            make_literal(repository, free, [left_term]),
            make_literal(repository, free, [right_term]),
        ]
        goals: list[pypddl.ConditionLiteral] = []
        for ball in ball_objects:
            ball_term = ball
            initial_literals.append(make_literal(repository, at, [ball_term, rooma_term]))
            goals.append(make_condition(repository, at, [ball_term, roomb_term]))

        return repository.insert(
            pypddl.TaskData(
                name,
                domain.index(),
                requirements=[],
                types=[],
                predicates=[],
                objects=[
                    rooma.index(),
                    roomb.index(),
                    left.index(),
                    right.index(),
                    *(each.index() for each in ball_objects),
                ],
                initial_literals=[each.index() for each in initial_literals],
                initial_function_values=[],
                goal=(make_condition_and(repository, goals)).index(),
                metric=None,
                axioms=[],
            ),
        )[0]

    one_ball = task("gripper-1-ball", ["ball1"])
    two_ball = task("gripper-2-ball", ["ball1", "ball2"])
    return repository, domain, one_ball, two_ball


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
