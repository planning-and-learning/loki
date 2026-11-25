(define (domain choice-nested)
  (:requirements :typing :non-deterministic :probabilistic-effects :conditional-effects)
  (:types item)
  (:predicates (a ?x - item) (p ?x - item) (q ?x - item) (r) (s) (t))
  ; (r) 0.45, (s) 0.25, nothing 0.3.
  (:action nested-probabilistic
    :parameters ()
    :effect (probabilistic 0.5 (probabilistic 0.5 (r) 0.5 (s)) 0.2 (r)))
  ; (r), (s) or (t).
  (:action nested-oneof
    :parameters ()
    :effect (oneof (r) (oneof (s) (t))))
  ; The boxworld pattern: a choice per item inside an outcome.
  (:action choice-in-outcome
    :parameters ()
    :effect (probabilistic 0.2 (forall (?x - item) (when (a ?x) (probabilistic 0.5 (p ?x) 0.5 (q ?x)))) 0.8 (r)))
)
