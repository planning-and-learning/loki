(define (domain adl-requirements)
  (:requirements :adl)
  (:predicates (p) (q))
  (:action a
    :parameters ()
    :precondition (or (p) (q))
    :effect (and))
)
