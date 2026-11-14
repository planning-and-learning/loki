(define (domain action-cost-implicit)
  (:requirements :action-costs)
  (:predicates (p))
  (:action pay
    :parameters ()
    :effect (increase (total-cost) 1))
)
