(define (domain numeric-facade)
  (:requirements :strips :fluents)
  (:predicates (ready))
  (:functions (total-cost))
  (:action spend
    :parameters ()
    :precondition (ready)
    :effect (and (ready) (increase (total-cost) 1)))
)
