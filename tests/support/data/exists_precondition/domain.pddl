(define (domain exists)
  (:requirements :typing :existential-preconditions)
  (:predicates (p ?x - object ?y - object) (q ?x - object))
  (:action a
    :parameters (?x - object)
    :precondition (and (q ?x) (exists (?y - object) (p ?x ?y)))
    :effect (and))
)
