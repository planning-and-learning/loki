(define (domain split-conditional-effect)
  (:requirements :strips :conditional-effects :disjunctive-preconditions)
  (:predicates (p ?x) (q ?x) (done ?x))
  (:action a
    :parameters (?x)
    :precondition ()
    :effect (when (or (p ?x) (q ?x)) (done ?x)))
)
