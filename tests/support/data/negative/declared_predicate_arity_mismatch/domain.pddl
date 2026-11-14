(define (domain bad-arity)
  (:predicates (p ?x))
  (:action a :parameters () :precondition (p) :effect (and))
)
