(define (domain universal-name-collision)
  (:requirements :strips :typing :universal-preconditions)
  (:types thing)
  (:predicates (loki-universal-0) (p ?x - thing) (done))
  (:action a
    :parameters ()
    :precondition (forall (?x - thing) (p ?x))
    :effect (done))
)
