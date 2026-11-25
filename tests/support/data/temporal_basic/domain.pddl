(define (domain temporal-basic)
  (:requirements :strips :typing :disjunctive-preconditions :existential-preconditions :universal-preconditions
                 :conditional-effects :numeric-fluents :durative-actions :duration-inequalities)
  (:types truck location)
  (:predicates (at ?t - truck ?l - location) (road ?a ?b - location) (free ?l - location) (open ?l - location) (sunny))
  (:functions (distance ?a ?b - location) (fuel ?t - truck))
  ; The predicate at next to the time specifier at: (at start (at ?t ?from)).
  (:durative-action drive
    :parameters (?t - truck ?from ?to - location)
    :duration (and (>= ?duration (distance ?from ?to)) (<= ?duration (* 2 (distance ?from ?to))))
    :condition (and (at start (at ?t ?from))
                    (over all (road ?from ?to))
                    (at end (free ?to))
                    (forall (?l - location) (over all (or (open ?l) (sunny)))))
    :effect (and (at start (not (at ?t ?from)))
                 (at end (at ?t ?to))
                 (at end (decrease (fuel ?t) ?duration))
                 (when (at start (sunny)) (at end (free ?from)))
                 (when (at end (sunny)) (at end (open ?to)))
                 (forall (?l - location) (at end (when (open ?l) (free ?l))))))
  ; A disjunction over all and an existential at end: derived predicates, not split or lifted into parameters.
  (:durative-action wait
    :parameters (?t - truck)
    :duration (= ?duration 1)
    :condition (and (over all (or (sunny) (exists (?l - location) (at ?t ?l))))
                    (at end (and (sunny) (exists (?l - location) (free ?l)))))
    :effect (at end (sunny)))
)
