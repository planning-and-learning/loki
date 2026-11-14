(define (domain action-cost-reads)
  (:requirements :action-costs)
  (:predicates (p))
  (:functions (total-cost) (road-length))
  (:action drive
    :parameters ()
    :effect (increase (total-cost) (road-length)))
)
