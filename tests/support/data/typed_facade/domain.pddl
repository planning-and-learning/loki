(define (domain typed-facade)
  (:requirements :strips :typing)
  (:types base - object item - base)
  (:predicates (ready ?x - item))
  (:action mark
    :parameters (?x - item)
    :precondition (ready ?x)
    :effect (ready ?x))
)
