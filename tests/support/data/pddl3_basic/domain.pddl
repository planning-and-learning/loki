(define (domain pddl3-basic)
  (:requirements :strips :typing :negative-preconditions :disjunctive-preconditions :universal-preconditions
                 :preferences :constraints)
  (:types block)
  ; The predicates after the constants' natural place: sections in any order.
  (:predicates (on ?x ?y - block) (clear ?x - block) (held ?x - block) (fragile ?x - block))
  (:constants table - block)
  (:action pick
    :parameters (?x - block)
    :precondition (and (clear ?x) (preference careful (or (not (fragile ?x)) (clear table))))
    :effect (and (held ?x) (not (clear ?x))))
  (:constraints (and (forall (?x - block) (always (or (clear ?x) (held ?x) (fragile ?x))))
                     (sometime-before (held table) (clear table))
                     (preference tidy (at end (clear table)))))
)
