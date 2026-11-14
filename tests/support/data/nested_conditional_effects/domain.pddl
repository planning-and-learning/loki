(define (domain nested-conditional-effects)
  (:requirements :strips :conditional-effects)
  (:predicates (p ?x) (q ?x) (done ?x))
  (:action a
    :parameters (?x)
    :precondition ()
    :effect (when (p ?x) (when (q ?x) (done ?x))))
)
