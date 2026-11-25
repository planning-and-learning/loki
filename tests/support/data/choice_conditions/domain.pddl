(define (domain choice-conditions)
  (:requirements :typing :non-deterministic :conditional-effects :disjunctive-preconditions :existential-preconditions)
  (:types item)
  (:predicates (a ?x - item) (b ?x - item) (p ?x - item) (q ?x - item) (r))
  ; Split into one conditional effect per disjunct, the choice would be drawn twice when both hold.
  (:action disjunctive
    :parameters (?x - item)
    :effect (when (or (a ?x) (b ?x)) (oneof (p ?x) (q ?x))))
  ; Lifted into a forall effect, the choice would be drawn once per witness.
  (:action existential
    :parameters ()
    :effect (when (exists (?y - item) (a ?y)) (oneof (r) (not (r)))))
  ; A deterministic effect is split as before.
  (:action deterministic
    :parameters (?x - item)
    :effect (when (or (a ?x) (b ?x)) (p ?x)))
)
