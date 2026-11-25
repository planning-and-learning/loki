(define (domain ppddl-rewards)
  (:requirements :strips :probabilistic-effects :rewards)
  (:predicates (p) (q))
  ; PPDDL: :parameters is optional, probabilities may be fractions, and :rewards declares (reward).
  (:action flip
    :effect (and (decrease (reward) 1) (probabilistic 1/3 (p) 2/3 (q))))
)
