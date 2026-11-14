(define (domain split-action-precondition)
  (:requirements :strips :disjunctive-preconditions)
  (:predicates (p) (q) (done))
  (:action a
    :parameters ()
    :precondition (or (p) (q))
    :effect (done))
)
