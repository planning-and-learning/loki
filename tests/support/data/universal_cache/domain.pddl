(define (domain universal-cache)
  (:requirements :typing :universal-preconditions)
  (:types left right)
  (:predicates (p ?x - object ?y - object))
  (:action a
    :parameters (?x - left)
    :precondition (forall (?y - object) (p ?x ?y))
    :effect (and))
  (:action b
    :parameters (?x - right)
    :precondition (forall (?y - object) (p ?x ?y))
    :effect (and))
)
