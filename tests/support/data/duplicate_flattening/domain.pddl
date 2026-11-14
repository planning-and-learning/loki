(define (domain duplicate-flattening)
  (:requirements :strips)
  (:predicates (p) (q))
  (:action a
    :parameters ()
    :precondition (and (p) (p))
    :effect (and (q) (q)))
)
