; Formulas over the state trajectory beyond PDDL 3 (:trajectory-formulas): nesting, the future, past and metric operators,
; negation, disjunction and existentials. The predicate next is an atom, not the operator: it takes no formula.
(define (domain trajectory-formulas)
  (:requirements :strips :typing :negative-preconditions :disjunctive-preconditions :existential-preconditions
                 :universal-preconditions :preferences :trajectory-formulas)
  (:types room)
  (:constants r1 r2 - room)
  (:predicates (at ?r - room) (visited ?r - room) (door-open) (next ?a ?b - room))
  (:action move
    :parameters (?a ?b - room)
    :precondition (and (at ?a) (next ?a ?b) (door-open))
    :effect (and (not (at ?a)) (at ?b) (visited ?b)))
  (:action open
    :parameters ()
    :precondition (not (door-open))
    :effect (door-open))
  (:constraints
    (and (always (sometime (door-open)))
         (forall (?r - room) (imply (visited ?r) (once (at ?r))))
         (not (sometime (and (door-open) (next (not (door-open))))))
         (until (interval 0 inf) (not (visited r2)) (visited r1))
         (exists (?r - room) (release (at ?r) (visited ?r)))
         (historically (interval 0 10) (or (door-open) (weak-yesterday (not (door-open)))))
         (triggers (door-open) (yesterday (door-open)))
         (weak-next (at r1))
         (preference reach (sometime (since (at r2) (visited r1))))))
)
