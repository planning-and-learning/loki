/*
 * Copyright (C) 2024-2026 Dominik Drexler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "bindings.hpp"

#include <loki/formalism/repository.hpp>
#include <nanobind/stl/shared_ptr.h>

namespace nb = nanobind;

namespace loki::formalism
{
namespace
{

void bind_formalism_enums(nb::module_& m)
{
    nb::enum_<formalism::BinaryComparator>(m, "BinaryComparator")
        .value("Eq", formalism::BinaryComparator::Eq)
        .value("Ne", formalism::BinaryComparator::Ne)
        .value("Lt", formalism::BinaryComparator::Lt)
        .value("Le", formalism::BinaryComparator::Le)
        .value("Gt", formalism::BinaryComparator::Gt)
        .value("Ge", formalism::BinaryComparator::Ge);

    nb::enum_<formalism::UnaryArithmeticOperator>(m, "UnaryArithmeticOperator").value("Sub", formalism::UnaryArithmeticOperator::Sub);

    nb::enum_<formalism::BinaryArithmeticOperator>(m, "BinaryArithmeticOperator")
        .value("Add", formalism::BinaryArithmeticOperator::Add)
        .value("Sub", formalism::BinaryArithmeticOperator::Sub)
        .value("Mul", formalism::BinaryArithmeticOperator::Mul)
        .value("Div", formalism::BinaryArithmeticOperator::Div);

    nb::enum_<formalism::MultiArithmeticOperator>(m, "MultiArithmeticOperator")
        .value("Add", formalism::MultiArithmeticOperator::Add)
        .value("Mul", formalism::MultiArithmeticOperator::Mul);

    nb::enum_<formalism::NumericEffectOperator>(m, "NumericEffectOperator")
        .value("Assign", formalism::NumericEffectOperator::Assign)
        .value("Increase", formalism::NumericEffectOperator::Increase)
        .value("Decrease", formalism::NumericEffectOperator::Decrease)
        .value("ScaleUp", formalism::NumericEffectOperator::ScaleUp)
        .value("ScaleDown", formalism::NumericEffectOperator::ScaleDown);

    nb::enum_<formalism::OptimizationDirection>(m, "OptimizationDirection")
        .value("Minimize", formalism::OptimizationDirection::Minimize)
        .value("Maximize", formalism::OptimizationDirection::Maximize);

    nb::enum_<formalism::RequirementKind>(m, "RequirementKind")
        .value("Strips", formalism::RequirementKind::Strips)
        .value("Typing", formalism::RequirementKind::Typing)
        .value("NegativePreconditions", formalism::RequirementKind::NegativePreconditions)
        .value("DisjunctivePreconditions", formalism::RequirementKind::DisjunctivePreconditions)
        .value("Equality", formalism::RequirementKind::Equality)
        .value("ExistentialPreconditions", formalism::RequirementKind::ExistentialPreconditions)
        .value("UniversalPreconditions", formalism::RequirementKind::UniversalPreconditions)
        .value("QuantifiedPreconditions", formalism::RequirementKind::QuantifiedPreconditions)
        .value("Adl", formalism::RequirementKind::Adl)
        .value("ConditionalEffects", formalism::RequirementKind::ConditionalEffects)
        .value("Fluents", formalism::RequirementKind::Fluents)
        .value("NumericFluents", formalism::RequirementKind::NumericFluents)
        .value("DurativeActions", formalism::RequirementKind::DurativeActions)
        .value("DerivedPredicates", formalism::RequirementKind::DerivedPredicates)
        .value("NonDeterministic", formalism::RequirementKind::NonDeterministic)
        .value("ProbabilisticEffects", formalism::RequirementKind::ProbabilisticEffects);
}

}  // namespace

void bind_formalism(nb::module_& m)
{
    bind_formalism_enums(m);

    auto repository = RepositoryBinding(m, "Repository", "Owns interned formalism objects created from builder data.");
    bind_requirement(m, repository);
    bind_type(m, repository);
    bind_object(m, repository);
    bind_variable(m, repository);
    bind_parameter(m, repository);
    bind_predicate(m, repository);
    bind_function_skeleton(m, repository);
    bind_term(m, repository);
    bind_atom(m, repository);
    bind_literal(m, repository);
    bind_function_expression_number(m, repository);
    bind_function_term(m, repository);
    bind_unary_function_expression(m, repository);
    bind_binary_function_expression(m, repository);
    bind_multi_function_expression(m, repository);
    bind_function_expression(m, repository);
    bind_condition_literal(m, repository);
    bind_condition_and(m, repository);
    bind_condition_or(m, repository);
    bind_condition_not(m, repository);
    bind_condition_imply(m, repository);
    bind_condition_exists(m, repository);
    bind_condition_forall(m, repository);
    bind_condition_numeric_constraint(m, repository);
    bind_condition(m, repository);
    bind_effect_literal(m, repository);
    bind_effect_and(m, repository);
    bind_effect_numeric(m, repository);
    bind_effect_forall(m, repository);
    bind_effect_when(m, repository);
    bind_effect_one_of(m, repository);
    bind_effect_probabilistic_alternative(m, repository);
    bind_effect_probabilistic(m, repository);
    bind_effect(m, repository);
    bind_action(m, repository);
    bind_axiom(m, repository);
    bind_metric(m, repository);
    bind_initial_function_value(m, repository);
    bind_domain(m, repository);
    bind_task(m, repository);

    nb::class_<RepositoryFactory>(m, "RepositoryFactory", "Factory for creating shared formalism repositories.")
        .def(nb::init<>())
        .def("create", [](RepositoryFactory& self) { return self.create_shared(); }, "Create a repository that owns interned formalism objects.");
}

}  // namespace loki::formalism
