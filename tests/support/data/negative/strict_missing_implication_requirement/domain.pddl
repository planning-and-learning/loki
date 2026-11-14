(define (domain missing-implication-requirement)
  (:requirements :strips)
  (:action a :parameters () :precondition (imply (p) (q)) :effect (and))
)
