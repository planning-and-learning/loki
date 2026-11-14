(define (domain variable-renaming)
  (:requirements :strips :typing :existential-preconditions :universal-preconditions)
  (:types thing)
  (:predicates (p ?x - thing) (q ?x - thing) (done ?x - thing))
  (:action a
    :parameters (?x - thing)
    :precondition (exists (?x - thing) (and (p ?x) (forall (?x - thing) (q ?x))))
    :effect (forall (?x - thing) (done ?x)))
)
