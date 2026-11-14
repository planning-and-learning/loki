(define (domain negated-exists)
  (:requirements :typing :existential-preconditions)
  (:predicates (p ?x - object ?y - object))
  (:action a
    :parameters (?x - object)
    :precondition (not (exists (?y - object) (p ?x ?y)))
    :effect (and))
)
