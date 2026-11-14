(define (domain nnf)
  (:predicates (p) (q) (r))
  (:action a
    :parameters ()
    :precondition (imply (not (and (p) (q))) (r))
    :effect (and (p)))
)
