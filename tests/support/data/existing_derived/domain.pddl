(define (domain existing-derived)
  (:requirements :strips)
  (:predicates (p ?x) (q ?x))
  (:derived (q ?x) (p ?x))
  (:action a
    :parameters (?x)
    :precondition (q ?x)
    :effect (p ?x))
)
