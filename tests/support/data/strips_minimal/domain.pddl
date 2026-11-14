(define (domain test-domain)
  (:requirements :strips)
  (:predicates (p))
  (:action a
    :parameters ()
    :precondition (and)
    :effect (and (p)))
)
