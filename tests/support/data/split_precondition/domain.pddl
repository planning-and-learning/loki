(define (domain split)
  (:requirements :disjunctive-preconditions)
  (:predicates (p) (q) (r))
  (:action a
    :parameters ()
    :precondition (and (p) (or (q) (r)))
    :effect (and))
)
