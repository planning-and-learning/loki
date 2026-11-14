(define (domain conditional-multiply)
  (:requirements :strips :conditional-effects :negative-preconditions)
  (:predicates (p) (q) (r) (s) (t))
  (:action a
    :parameters ()
    :precondition (p)
    :effect (and (when (q) (r)) (when (s) (t))))
)
