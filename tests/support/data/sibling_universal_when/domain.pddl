(define (domain sibling-universal-when)
  (:requirements :universal-preconditions :conditional-effects)
  (:predicates (p ?x) (q ?x) (a) (b))
  (:action act
    :parameters ()
    :precondition ()
    :effect (when (forall (?x) (p ?x)) (and (a) (when (forall (?y) (q ?y)) (b)))))
)
