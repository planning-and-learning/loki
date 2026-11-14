(define (domain effects)
  (:requirements :conditional-effects :disjunctive-preconditions)
  (:predicates (p) (q) (r) (s))
  (:action a
    :parameters ()
    :precondition ()
    :effect (when (or (p) (q)) (and (r) (s))))
)
