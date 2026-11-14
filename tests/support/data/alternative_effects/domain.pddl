(define (domain facade-format-alternatives)
  (:requirements :strips :non-deterministic :probabilistic-effects)
  (:predicates (p) (q))
  (:action choose
    :parameters ()
    :effect (oneof (p) (q)))
  (:action sample
    :parameters ()
    :effect (probabilistic 0.25 (p) 0.75 (q)))
)
