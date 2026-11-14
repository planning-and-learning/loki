(define (domain duplicate-action-name)
  (:predicates (p) (q))
  (:action a
    :parameters ()
    :effect (p))
  (:action a
    :parameters ()
    :effect (q))
  (:action a
    :parameters ()
    :effect (p))
)
