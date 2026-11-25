; A type is a unary predicate: the predicate fault over a fault defines it a second time (as the IPC 2008 FOND faults
; domain did). Only a unary predicate over the root, the type declared again, may take its name.
(define (domain predicate-redefines-type)
    (:requirements :strips :typing)
    (:types fault)
    (:predicates (fault ?f - fault))
    (:action clear-fault
        :parameters (?f - fault)
        :precondition (fault ?f)
        :effect (not (fault ?f))))
