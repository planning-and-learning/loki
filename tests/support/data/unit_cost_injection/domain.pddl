(define (domain plain-domain)
  (:requirements :strips)
  (:predicates (p))
  (:action flip
    :parameters ()
    :effect (p))
)
