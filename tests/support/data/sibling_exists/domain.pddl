(define (domain sibling-exists)
  (:requirements :existential-preconditions)
  (:predicates (a ?x) (b ?x) (done))
  (:action act
    :parameters ()
    :precondition (and (exists (?p) (a ?p)) (exists (?p) (b ?p)))
    :effect (done))
)
