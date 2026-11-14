(define (domain adl-equality)
  (:requirements :adl)
  (:predicates (p ?x))
  (:action a
    :parameters (?x)
    :precondition (= ?x ?x)
    :effect (p ?x))
)
