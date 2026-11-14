(define (domain lift-capture)
  (:requirements :existential-preconditions)
  (:predicates (a ?x) (b ?x ?y) (done))
  (:action act
    :parameters (?p_0)
    :precondition (and (exists (?p) (a ?p)) (exists (?p) (b ?p ?p_0)))
    :effect (done))
)
