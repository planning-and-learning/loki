(define (domain rename-once)
  (:requirements :typing :existential-preconditions)
  (:predicates (p ?x - object ?y - object))
  (:action a
    :parameters (?x - object)
    :precondition (not (exists (?x - object) (p ?x ?x)))
    :effect (and))
)
