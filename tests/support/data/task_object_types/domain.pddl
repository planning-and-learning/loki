(define (domain task-object-types)
  (:requirements :strips :typing)
  (:types bread-portion content-portion place)
  (:predicates (ready))
  (:action mark-ready
    :parameters ()
    :precondition (and)
    :effect (ready)))
