(define (domain permissive-requirements)
  (:requirements :strips)
  (:predicates (p) (q))
  (:action a
    :parameters ()
    :precondition (or (p) (q))
    :effect (and))
)
