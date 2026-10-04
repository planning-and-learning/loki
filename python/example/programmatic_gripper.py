"""Build a gripper domain and two tasks programmatically with pypddl."""

from collections.abc import Sequence

from pypddl import formalism as pypddl


def make_literal(
    repository: pypddl.Repository,
    predicate: pypddl.Predicate,
    terms: Sequence[pypddl.Term],
    positive: bool = True,
) -> pypddl.Literal:
    atom, _ = repository.insert(pypddl.AtomData(predicate, list(terms)))
    return repository.insert(pypddl.LiteralData(atom, positive))[0]


def make_condition(
    repository: pypddl.Repository,
    predicate: pypddl.Predicate,
    terms: Sequence[pypddl.Term],
    positive: bool = True,
) -> pypddl.Condition:
    literal = make_literal(repository, predicate, terms, positive)
    condition_literal, _ = repository.insert(pypddl.ConditionLiteralData(literal))
    return repository.insert(pypddl.ConditionData(condition_literal))[0]


def make_effect(repository: pypddl.Repository, literal: pypddl.Literal) -> pypddl.Effect:
    effect_literal, _ = repository.insert(pypddl.EffectLiteralData(literal))
    return repository.insert(pypddl.EffectData(effect_literal))[0]


def make_condition_and(
    repository: pypddl.Repository,
    conditions: Sequence[pypddl.Condition],
) -> pypddl.Condition:
    conjunction, _ = repository.insert(pypddl.ConditionAndData(list(conditions)))
    return repository.insert(pypddl.ConditionData(conjunction))[0]


def make_effect_and(
    repository: pypddl.Repository,
    effects: Sequence[pypddl.Effect],
) -> pypddl.Effect:
    conjunction, _ = repository.insert(pypddl.EffectAndData(list(effects)))
    return repository.insert(pypddl.EffectData(conjunction))[0]


def build_gripper() -> tuple[pypddl.Repository, pypddl.Domain, pypddl.Task, pypddl.Task]:
    repository = pypddl.RepositoryFactory().create()
    strips, _ = repository.insert(pypddl.RequirementData(pypddl.RequirementKind.Strips))
    typing, _ = repository.insert(pypddl.RequirementData(pypddl.RequirementKind.Typing))

    object_t, _ = repository.insert(pypddl.TypeData("object"))
    room_t, _ = repository.insert(pypddl.TypeData("room", [object_t]))
    ball_t, _ = repository.insert(pypddl.TypeData("ball", [object_t]))
    gripper_t, _ = repository.insert(pypddl.TypeData("gripper", [object_t]))

    x, _ = repository.insert(
        pypddl.ParameterData(repository.insert(pypddl.VariableData("?x"))[0], [room_t])
    )
    y, _ = repository.insert(
        pypddl.ParameterData(repository.insert(pypddl.VariableData("?y"))[0], [room_t])
    )
    b, _ = repository.insert(
        pypddl.ParameterData(repository.insert(pypddl.VariableData("?b"))[0], [ball_t])
    )
    g, _ = repository.insert(
        pypddl.ParameterData(repository.insert(pypddl.VariableData("?g"))[0], [gripper_t])
    )

    at_robby, _ = repository.insert(pypddl.PredicateData("at-robby", [x]))
    at, _ = repository.insert(pypddl.PredicateData("at", [b, x]))
    free, _ = repository.insert(pypddl.PredicateData("free", [g]))
    carry, _ = repository.insert(pypddl.PredicateData("carry", [b, g]))

    x_term, _ = repository.insert(pypddl.TermData(x.get_variable()))
    y_term, _ = repository.insert(pypddl.TermData(y.get_variable()))
    b_term, _ = repository.insert(pypddl.TermData(b.get_variable()))
    g_term, _ = repository.insert(pypddl.TermData(g.get_variable()))

    move_pre = make_condition(repository, at_robby, [x_term])
    move_eff = make_effect_and(repository, [
        make_effect(repository, make_literal(repository, at_robby, [x_term], False)),
        make_effect(repository, make_literal(repository, at_robby, [y_term])),
    ])
    move, _ = repository.insert(pypddl.ActionData("move", [x, y], move_pre, move_eff))

    pick_pre = make_condition_and(repository, [
        make_condition(repository, at, [b_term, x_term]),
        make_condition(repository, at_robby, [x_term]),
        make_condition(repository, free, [g_term]),
    ])
    pick_eff = make_effect_and(repository, [
        make_effect(repository, make_literal(repository, at, [b_term, x_term], False)),
        make_effect(repository, make_literal(repository, free, [g_term], False)),
        make_effect(repository, make_literal(repository, carry, [b_term, g_term])),
    ])
    pick, _ = repository.insert(pypddl.ActionData("pick", [b, x, g], pick_pre, pick_eff))

    drop_pre = make_condition_and(repository, [
        make_condition(repository, carry, [b_term, g_term]),
        make_condition(repository, at_robby, [x_term]),
    ])
    drop_eff = make_effect_and(repository, [
        make_effect(repository, make_literal(repository, carry, [b_term, g_term], False)),
        make_effect(repository, make_literal(repository, free, [g_term])),
        make_effect(repository, make_literal(repository, at, [b_term, x_term])),
    ])
    drop, _ = repository.insert(pypddl.ActionData("drop", [b, x, g], drop_pre, drop_eff))

    domain, _ = repository.insert(
        pypddl.DomainData(
            "gripper",
            requirements=[strips, typing],
            types=[object_t, room_t, ball_t, gripper_t],
            predicates=[at_robby, at, free, carry],
            actions=[move, pick, drop],
        ),
    )

    def task(name: str, balls: Sequence[str]) -> pypddl.Task:
        rooma, _ = repository.insert(pypddl.ObjectData("rooma", [room_t]))
        roomb, _ = repository.insert(pypddl.ObjectData("roomb", [room_t]))
        left, _ = repository.insert(pypddl.ObjectData("left", [gripper_t]))
        right, _ = repository.insert(pypddl.ObjectData("right", [gripper_t]))
        ball_objects = [repository.insert(pypddl.ObjectData(ball, [ball_t]))[0] for ball in balls]

        rooma_term, _ = repository.insert(pypddl.TermData(rooma))
        roomb_term, _ = repository.insert(pypddl.TermData(roomb))
        left_term, _ = repository.insert(pypddl.TermData(left))
        right_term, _ = repository.insert(pypddl.TermData(right))

        initial_literals = [
            make_literal(repository, at_robby, [rooma_term]),
            make_literal(repository, free, [left_term]),
            make_literal(repository, free, [right_term]),
        ]
        goals: list[pypddl.Condition] = []
        for ball in ball_objects:
            ball_term, _ = repository.insert(pypddl.TermData(ball))
            initial_literals.append(make_literal(repository, at, [ball_term, rooma_term]))
            goals.append(make_condition(repository, at, [ball_term, roomb_term]))

        return repository.insert(
            pypddl.TaskData(
                name,
                domain,
                objects=[rooma, roomb, left, right, *ball_objects],
                initial_literals=initial_literals,
                goal=make_condition_and(repository, goals),
            ),
        )[0]

    one_ball = task("gripper-1-ball", ["ball1"])
    two_ball = task("gripper-2-ball", ["ball1", "ball2"])
    return repository, domain, one_ball, two_ball


if __name__ == "__main__":
    _, domain, one_ball, two_ball = build_gripper()
    print(f"domain: {domain.get_name()}, actions={len(domain.get_actions())}, predicates={len(domain.get_predicates())}")
    print(f"task: {one_ball.get_name()}, objects={len(one_ball.get_objects())}, init={len(one_ball.get_initial_literals())}")
    print(f"task: {two_ball.get_name()}, objects={len(two_ball.get_objects())}, init={len(two_ball.get_initial_literals())}")
