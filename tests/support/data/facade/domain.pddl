(define (domain facade)
  (:requirements :strips)
  (:predicates (ready))
  (:action wait
    :parameters ()
    :precondition (ready)
    :effect (ready))
)
