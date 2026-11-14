(define (domain rename)
  (:requirements :typing :existential-preconditions)
  (:predicates (p ?x - object))
  (:action a
    :parameters (?x - object)
    :precondition (exists (?x - object) (p ?x))
    :effect (and (p ?x)))
)
