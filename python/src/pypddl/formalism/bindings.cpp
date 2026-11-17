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

#include "pypddl/formalism/module.hpp"

#include <loki/formalism/repository.hpp>
#include <loki/formalism/views.hpp>
#include <nanobind/stl/shared_ptr.h>
#include <string>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace nb = nanobind;
using namespace nb::literals;

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
        .value("ActionCosts", formalism::RequirementKind::ActionCosts)
        .value("DurativeActions", formalism::RequirementKind::DurativeActions)
        .value("DerivedPredicates", formalism::RequirementKind::DerivedPredicates)
        .value("NonDeterministic", formalism::RequirementKind::NonDeterministic)
        .value("ProbabilisticEffects", formalism::RequirementKind::ProbabilisticEffects);
}

template<typename T>
void bind_insert(RepositoryBinding& repository)
{
    // The record is canonicalized where it is (its lists sorted) and interned: records are not copied.
    repository.def("insert", [](Repository& self, urd::data<T>& data) { return formalism::insert(self, data); }, nb::arg("data"));
}

/// Every record of SymbolRepositoryTypes, in its order: <Name>Index and <Name>Data (bind_data), the view <Name>
/// (bind_view) and Repository.insert of its data.
void bind_records(nb::module_& m, RepositoryBinding& repository)
{
    urd::python::bind_data<Requirement>(m);
    urd::python::bind_view<EntityView<Requirement>>(m);
    bind_insert<Requirement>(repository);

    urd::python::bind_data<Type>(m);
    urd::python::bind_view<EntityView<Type>>(m);
    bind_insert<Type>(repository);

    urd::python::bind_data<Object>(m);
    urd::python::bind_view<EntityView<Object>>(m);
    bind_insert<Object>(repository);

    urd::python::bind_data<Variable>(m);
    urd::python::bind_view<EntityView<Variable>>(m);
    bind_insert<Variable>(repository);

    urd::python::bind_data<Parameter>(m);
    urd::python::bind_view<EntityView<Parameter>>(m);
    bind_insert<Parameter>(repository);

    urd::python::bind_data<Predicate>(m);
    urd::python::bind_view<EntityView<Predicate>>(m);
    bind_insert<Predicate>(repository);

    urd::python::bind_data<FunctionSkeleton>(m);
    urd::python::bind_view<EntityView<FunctionSkeleton>>(m);
    bind_insert<FunctionSkeleton>(repository);

    urd::python::bind_data<Atom>(m);
    urd::python::bind_view<EntityView<Atom>>(m);
    bind_insert<Atom>(repository);

    urd::python::bind_data<Literal>(m);
    urd::python::bind_view<EntityView<Literal>>(m);
    bind_insert<Literal>(repository);

    urd::python::bind_data<FunctionExpression<NumberTag>>(m);
    urd::python::bind_view<EntityView<FunctionExpression<NumberTag>>>(m);
    bind_insert<FunctionExpression<NumberTag>>(repository);

    urd::python::bind_data<FunctionTerm>(m);
    urd::python::bind_view<EntityView<FunctionTerm>>(m);
    bind_insert<FunctionTerm>(repository);

    urd::python::bind_data<FunctionExpression<UnaryTag>>(m);
    urd::python::bind_view<EntityView<FunctionExpression<UnaryTag>>>(m);
    bind_insert<FunctionExpression<UnaryTag>>(repository);

    urd::python::bind_data<FunctionExpression<BinaryTag>>(m);
    urd::python::bind_view<EntityView<FunctionExpression<BinaryTag>>>(m);
    bind_insert<FunctionExpression<BinaryTag>>(repository);

    urd::python::bind_data<FunctionExpression<MultiTag>>(m);
    urd::python::bind_view<EntityView<FunctionExpression<MultiTag>>>(m);
    bind_insert<FunctionExpression<MultiTag>>(repository);

    urd::python::bind_data<Condition<LiteralTag>>(m);
    urd::python::bind_view<EntityView<Condition<LiteralTag>>>(m);
    bind_insert<Condition<LiteralTag>>(repository);

    urd::python::bind_data<Condition<AndTag>>(m);
    urd::python::bind_view<EntityView<Condition<AndTag>>>(m);
    bind_insert<Condition<AndTag>>(repository);

    urd::python::bind_data<Condition<OrTag>>(m);
    urd::python::bind_view<EntityView<Condition<OrTag>>>(m);
    bind_insert<Condition<OrTag>>(repository);

    urd::python::bind_data<Condition<NotTag>>(m);
    urd::python::bind_view<EntityView<Condition<NotTag>>>(m);
    bind_insert<Condition<NotTag>>(repository);

    urd::python::bind_data<Condition<ImplyTag>>(m);
    urd::python::bind_view<EntityView<Condition<ImplyTag>>>(m);
    bind_insert<Condition<ImplyTag>>(repository);

    urd::python::bind_data<Condition<ExistsTag>>(m);
    urd::python::bind_view<EntityView<Condition<ExistsTag>>>(m);
    bind_insert<Condition<ExistsTag>>(repository);

    urd::python::bind_data<Condition<ForallTag>>(m);
    urd::python::bind_view<EntityView<Condition<ForallTag>>>(m);
    bind_insert<Condition<ForallTag>>(repository);

    urd::python::bind_data<Condition<NumericConstraintTag>>(m);
    urd::python::bind_view<EntityView<Condition<NumericConstraintTag>>>(m);
    bind_insert<Condition<NumericConstraintTag>>(repository);

    urd::python::bind_data<Effect<LiteralTag>>(m);
    urd::python::bind_view<EntityView<Effect<LiteralTag>>>(m);
    bind_insert<Effect<LiteralTag>>(repository);

    urd::python::bind_data<Effect<AndTag>>(m);
    urd::python::bind_view<EntityView<Effect<AndTag>>>(m);
    bind_insert<Effect<AndTag>>(repository);

    urd::python::bind_data<Effect<NumericTag>>(m);
    urd::python::bind_view<EntityView<Effect<NumericTag>>>(m);
    bind_insert<Effect<NumericTag>>(repository);

    urd::python::bind_data<Effect<ForallTag>>(m);
    urd::python::bind_view<EntityView<Effect<ForallTag>>>(m);
    bind_insert<Effect<ForallTag>>(repository);

    urd::python::bind_data<Effect<WhenTag>>(m);
    urd::python::bind_view<EntityView<Effect<WhenTag>>>(m);
    bind_insert<Effect<WhenTag>>(repository);

    urd::python::bind_data<Effect<OneOfTag>>(m);
    urd::python::bind_view<EntityView<Effect<OneOfTag>>>(m);
    bind_insert<Effect<OneOfTag>>(repository);

    urd::python::bind_data<EffectProbabilisticAlternative>(m);
    urd::python::bind_view<EntityView<EffectProbabilisticAlternative>>(m);
    bind_insert<EffectProbabilisticAlternative>(repository);

    urd::python::bind_data<Effect<ProbabilisticTag>>(m);
    urd::python::bind_view<EntityView<Effect<ProbabilisticTag>>>(m);
    bind_insert<Effect<ProbabilisticTag>>(repository);

    urd::python::bind_data<Action>(m);
    urd::python::bind_view<EntityView<Action>>(m);
    bind_insert<Action>(repository);

    urd::python::bind_data<Axiom>(m);
    urd::python::bind_view<EntityView<Axiom>>(m);
    bind_insert<Axiom>(repository);

    urd::python::bind_data<Metric>(m);
    urd::python::bind_view<EntityView<Metric>>(m);
    bind_insert<Metric>(repository);

    urd::python::bind_data<InitialFunctionValue>(m);
    urd::python::bind_view<EntityView<InitialFunctionValue>>(m);
    bind_insert<InitialFunctionValue>(repository);

    urd::python::bind_data<Domain>(m);
    urd::python::bind_view<EntityView<Domain>>(m);
    bind_insert<Domain>(repository);

    urd::python::bind_data<Task>(m);
    urd::python::bind_view<EntityView<Task>>(m);
    bind_insert<Task>(repository);
}

}  // namespace

void bind_formalism(nb::module_& m)
{
    bind_formalism_enums(m);

    auto repository = RepositoryBinding(m, "Repository", "Owns interned formalism objects created from builder data.");
    repository.def(nb::init<>(), "Create a repository that owns interned formalism objects.");
    bind_records(m, repository);
}

}  // namespace loki::formalism
