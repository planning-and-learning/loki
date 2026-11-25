(define (domain choice-forall)
  (:requirements :typing :non-deterministic :conditional-effects)
  (:types item)
  (:predicates (a ?x - item) (b ?x - item) (p ?x - item))
  ; One independent choice per item: the forall stays outside the choice.
  (:action flip-all
    :parameters ()
    :effect (forall (?x - item) (and (p ?x) (oneof (a ?x) (b ?x)))))
)
