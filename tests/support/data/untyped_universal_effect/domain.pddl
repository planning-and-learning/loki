(define (domain untyped-universal-effect)
  (:requirements :strips :universal-preconditions)
  (:predicates (p ?x))
  (:action a
    :parameters ()
    :precondition ()
    :effect (forall (?x) (p ?x)))
)
