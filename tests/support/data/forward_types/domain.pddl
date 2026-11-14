(define (domain forward-types)
  (:requirements :strips :typing)
  (:types child - parent parent - object)
  (:predicates (holds ?x - child)))
