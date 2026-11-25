# Loki's language

What loki reads beyond PDDL's classical core, and how its translator represents it: choices (nondeterministic and
probabilistic effects), the syntax of the competitions' benchmarks, PDDL 3's constraints and preferences, loki's
formulas over the state trajectory, and durative actions.

## Nondeterministic and probabilistic effects

A `oneof` or `probabilistic` effect is a choice, drawn once per application of the effect that holds it and
independently of every other choice (PPDDL 1.0, NPDDL). In particular, `(forall (?x) (oneof A B))` draws one
choice per object, and two equal choices in one conjunction are two draws. The translator therefore never
moves a choice across a `forall` and never distributes a conjunction over a choice. It normalizes the effects
into a factored form, a conjunction of independent pieces:

```
Effect ::= and(Piece*)
Piece  ::= forall(parameters, when(condition, Body))  ; parameters and condition may be empty
Body   ::= effect normal form | choice over outcomes, each an Effect
```

- A choice directly inside a choice is flattened (probabilities multiplied); equal outcomes are merged
  (probabilities added); the probability mass that the outcomes leave is an explicit empty outcome `(and)`.
- The condition of a conditional effect that holds a choice is a conjunction of literals. A disjunctive or
  existential condition becomes a generated derived predicate `loki-choice-*`: splitting it per disjunct or
  lifting it into a `forall` effect would draw the choice once per disjunct or witness.
- With `compile_conditional_effects`, the conditional effects inside outcomes are compiled as well.

## Syntax of the competitions

Besides PDDL's own grammar, loki reads what the competitions' benchmarks use:

- the sections of a domain in any order (constants after predicates, functions before types);
- a 0-ary function written bare as an effect's target, `(increase total-fuel-used 5)` (PDDL 2.1);
- an action without `:parameters`, probabilities written as fractions `1/3`, and `:rewards` with the fluent `(reward)`,
  declared implicitly, and `(:goal-reward n)` (PPDDL).

## Constraints and preferences

PDDL 3's `(:constraints ...)` of a domain or task, `(preference name c)` in preconditions, goals and constraints, and
`(is-violated name)` in metrics are represented as written. The translator compiles nothing away:

- each condition of a modal constraint (`always`, `within`, `sometime-before`, ...) stays one formula; each conjunct that
  is no literal or numeric constraint becomes a generated derived predicate `loki-constraint-*`;
- a preference's condition becomes a conjunction the same way (`loki-preference-*`): a preference is never split, since
  `is-violated` counts its violations;
- typing guards a universal constraint or preference with the types of its parameters (`loki-typed-*`), so that it holds
  vacuously outside them.

## Formulas over the state trajectory

With `:trajectory-formulas` (loki's extension; every PDDL 3 file stays valid), constraints and goals are formulas of
linear temporal logic over finite traces, past and metric operators included:

- PDDL 3's ten operators nest, e.g. `(always (sometime φ))`, and `and`, `or`, `not`, `imply`, `forall`, `exists`
  combine formulas;
- future: `(next φ)`, `(weak-next φ)`, `(until φ ψ)`, `(release φ ψ)` (`sometime` and `always` are F and G);
- past: `(yesterday φ)`, `(weak-yesterday φ)`, `(since φ ψ)`, `(triggers φ ψ)`, `(once φ)`, `(historically φ)`;
- metric: `until`, `release`, `since`, `triggers`, `once` and `historically` take an interval before their operands,
  `(until (interval 2 5) φ ψ)`, with `0 <= lo <= hi` and `hi` possibly `inf`;
- a goal description in a formula holds at the position it is read at; a formula is read at the initial state, so
  `(at end φ)` reads the final state;
- a goal with a modal operator is a formula the plan's trace satisfies: a hard constraint of the task, conjoined with
  its constraints (its preferences are soft ones); a goal without one is the final-state goal, as always;
- the semantics is that of finite traces (De Giacomo and Vardi, 2013): `weak-next` holds in the last state,
  `weak-yesterday` in the first; a metric interval bounds the time stamps of a temporal plan, and a classical plan's
  states are its time points;
- `or` and `imply` over formulas require `:disjunctive-preconditions`, `exists` `:existential-preconditions`, as `forall`
  requires `:universal-preconditions`; preferences stand only under `and` and `forall`.

The translator keeps formulas as written but for the negation normal form, which pushes `not` through the duals
(`next`/`weak-next`, `until`/`release`, `yesterday`/`weak-yesterday`, `since`/`triggers`, `once`/`historically`,
`sometime`/`always`, `at end`) and stops before PDDL 3's other operators. Goal descriptions become conjunctions as in
constraints (`loki-constraint-*`); compiling typing guards `(forall (?x - t) φ)` as `(or (loki-typed-* ?x) φ)` and
`(exists (?x - t) φ)` as `(and (t ?x) φ)`.

## Durative actions and timed initial literals

Loki supports PDDL 2.1 durative actions (level 3: discrete durative actions with duration constraints, including
duration inequalities) and PDDL 2.2 timed initial literals. Continuous effects (`#t`) are not supported.

The parser groups a durative action's condition and effect per time point:

- The condition is a conjunction of at most one `at start`, one `over all` and one `at end` condition, each untimed
  inside; `(forall (?x) (over all c))` becomes `(over all (forall (?x) c))`.
- The effect is a conjunction of at most one `at start` and one `at end` effect. The condition of a conditional
  effect that reads only its effect's time point is untimed; one that reads another time point (for example
  `(when (at start c) (at end e))`) keeps its timed conditions.

The translator never splits a durative action's condition or lifts its existentials into the action's parameters:
fixing one disjunct or one witness for an `over all` condition would demand that it hold throughout the interval.
Instead, each conjunct of a timed condition that is no literal or numeric constraint becomes a generated derived
predicate `loki-timed-*`. Typing adds the type conditions of the parameters to the `at start` condition.
`compile_conditional_effects` leaves durative actions as they are.
