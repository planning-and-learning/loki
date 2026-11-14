(define (domain universal)
  (:requirements :typing :universal-preconditions)
  (:predicates (p ?x - object ?y - object))
  (:action a
    :parameters (?x - object)
    :precondition (forall (?y - object) (p ?x ?y))
    :effect (and))
)
