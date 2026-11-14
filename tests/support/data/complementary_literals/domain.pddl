(define (domain contradiction)
  (:requirements :negative-preconditions)
  (:predicates (p) (q) (r))
  (:action a
    :parameters ()
    :precondition (and (p) (not (p)) (q))
    :effect (r))
)
