(define (domain facade-format)
  (:requirements :strips)
  (:predicates (ready) (seen ?x))
  (:action wait
    :parameters (?x)
    :precondition (ready)
    :effect (and (ready) (seen ?x)))
)
