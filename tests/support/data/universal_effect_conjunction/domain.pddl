(define (domain universal-effect-conjunction)
  (:requirements :strips :typing :universal-preconditions)
  (:types thing)
  (:predicates (p ?x - thing ?y - thing) (q ?x - thing ?y - thing))
  (:action a
    :parameters (?x - thing)
    :precondition ()
    :effect (forall (?y - thing) (and (p ?x ?y) (q ?x ?y))))
)
