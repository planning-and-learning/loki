(define (domain mixed-costs)
  (:requirements :strips)
  (:predicates (p))
  (:action expensive
    :parameters ()
    :effect (and (p) (increase (total-cost) 5)))
  (:action cheap
    :parameters ()
    :effect (p))
)
