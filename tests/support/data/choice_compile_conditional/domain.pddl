(define (domain choice-compile-conditional)
  (:requirements :strips :non-deterministic :conditional-effects)
  (:predicates (a) (p) (q))
  (:action choose
    :parameters ()
    :effect (oneof (when (a) (p)) (q)))
)
