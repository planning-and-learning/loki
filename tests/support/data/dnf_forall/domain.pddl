(define (domain dnf-forall)
  (:requirements :strips :typing :universal-preconditions :disjunctive-preconditions)
  (:types thing)
  (:predicates (p ?x - thing) (q ?x - thing) (done))
  (:action a
    :parameters ()
    :precondition (forall (?x - thing) (or (p ?x) (q ?x)))
    :effect (done))
)
