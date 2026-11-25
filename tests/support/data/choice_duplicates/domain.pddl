(define (domain choice-duplicates)
  (:requirements :strips :non-deterministic :probabilistic-effects)
  (:predicates (p) (q))
  ; Two equal oneofs are two independent choices: both stay.
  (:action flip-twice
    :parameters ()
    :effect (and (oneof (p) (q)) (oneof (p) (q))))
  ; Two equal alternatives add up: (p) with probability 1.
  (:action sample
    :parameters ()
    :effect (probabilistic 0.5 (p) 0.5 (p)))
)
