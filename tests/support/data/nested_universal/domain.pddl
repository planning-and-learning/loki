(define (domain nested-universal)
  (:requirements :strips :typing :universal-preconditions :existential-preconditions)
  (:types thing)
  (:predicates (p ?x - thing ?y - thing) (done ?x - thing))
  (:action a
    :parameters (?x - thing)
    :precondition (forall (?z - thing) (exists (?y - thing) (p ?z ?y)))
    :effect (done ?x))
)
