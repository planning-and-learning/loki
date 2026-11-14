(define (domain conditional-exists)
  (:requirements :strips :typing :conditional-effects :existential-preconditions)
  (:types thing)
  (:predicates (q ?x - thing ?y - thing) (done ?x - thing))
  (:action a
    :parameters (?x - thing)
    :precondition ()
    :effect (when (exists (?y - thing) (q ?x ?y)) (done ?x)))
)
