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

#include <boost/mp11/algorithm.hpp>
#include <loki/formalism/repository.hpp>
#include <loki/formalism/views.hpp>
#include <nanobind/stl/shared_ptr.h>
#include <ranges>
#include <string>
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
    repository.def("insert", [](Repository& self, urd::data<T> data) { return formalism::insert(self, data); }, nb::arg("data"));
}

/// A symbol type's Python class name (its index and data classes add "Index" and "Data") and the members of its data
/// class (constructors, fields) and of its view class (getters).
template<typename T>
struct Symbol;

template<>
struct Symbol<Requirement>
{
    static constexpr auto name = "Requirement";

    static void data(auto& cls)
    {
        using V = urd::data<Requirement>;
        cls.def(nb::init<RequirementKind>(), "kind"_a).def_rw("kind", &V::kind);
    }

    static void view(auto& cls)
    {
        using V = RequirementView;
        cls.def("get_kind", &V::get_kind);
    }
};

template<>
struct Symbol<Type>
{
    static constexpr auto name = "Type";

    static void data(auto& cls)
    {
        using V = urd::data<Type>;
        cls.def(nb::init<std::string, urd::flat_vector<urd::index<Type>>>(), "name"_a, "bases"_a = urd::flat_vector<urd::index<Type>> {})
            .def(urd::python::init_from_views<const std::string&, const urd::Store<urd::sequence<TypeView>>&>(), "name"_a, "bases"_a = urd::Store<urd::sequence<TypeView>> {})
            .def_rw("name", &V::name)
            .def_rw("bases", &V::bases);
    }

    static void view(auto& cls)
    {
        using V = TypeView;
        cls.def("get_name", &V::get_name).def("get_bases", &V::get_bases);
    }
};

template<>
struct Symbol<Object>
{
    static constexpr auto name = "Object";

    static void data(auto& cls)
    {
        using V = urd::data<Object>;
        cls.def(nb::init<std::string, urd::flat_vector<urd::index<Type>>>(), "name"_a, "types"_a = urd::flat_vector<urd::index<Type>> {})
            .def(urd::python::init_from_views<const std::string&, const urd::Store<urd::sequence<TypeView>>&>(), "name"_a, "types"_a = urd::Store<urd::sequence<TypeView>> {})
            .def_rw("name", &V::name)
            .def_rw("types", &V::types);
    }

    static void view(auto& cls)
    {
        using V = ObjectView;
        cls.def("get_name", &V::get_name).def("get_types", &V::get_types);
    }
};

template<>
struct Symbol<Variable>
{
    static constexpr auto name = "Variable";

    static void data(auto& cls)
    {
        using V = urd::data<Variable>;
        cls.def(nb::init<urd::flat_string>(), "name"_a).def_rw("name", &V::name);
    }

    static void view(auto& cls)
    {
        using V = VariableView;
        cls.def("get_name", &V::get_name);
    }
};

template<>
struct Symbol<Parameter>
{
    static constexpr auto name = "Parameter";

    static void data(auto& cls)
    {
        using V = urd::data<Parameter>;
        cls.def(nb::init<urd::index<Variable>, urd::flat_vector<urd::index<Type>>>(), "variable"_a, "types"_a = urd::flat_vector<urd::index<Type>> {})
            .def(urd::python::init_from_views<VariableView, const urd::Store<urd::sequence<TypeView>>&>(), "variable"_a, "types"_a = urd::Store<urd::sequence<TypeView>> {})
            .def_rw("variable", &V::variable)
            .def_rw("types", &V::types);
    }

    static void view(auto& cls)
    {
        using V = ParameterView;
        cls.def("get_variable", &V::get_variable).def("get_types", &V::get_types);
    }
};

template<>
struct Symbol<Predicate>
{
    static constexpr auto name = "Predicate";

    static void data(auto& cls)
    {
        using V = urd::data<Predicate>;
        cls.def(nb::init<std::string, urd::flat_vector<urd::index<Parameter>>>(), "name"_a, "parameters"_a = urd::flat_vector<urd::index<Parameter>> {})
            .def(urd::python::init_from_views<const std::string&, const urd::Store<urd::sequence<ParameterView>>&>(),
                 "name"_a,
                 "parameters"_a = urd::Store<urd::sequence<ParameterView>> {})
            .def_rw("name", &V::name)
            .def_rw("parameters", &V::parameters);
    }

    static void view(auto& cls)
    {
        using V = PredicateView;
        cls.def("get_name", &V::get_name).def("get_parameters", &V::get_parameters).def("get_arity", &V::get_arity);
    }
};

template<>
struct Symbol<FunctionSkeleton>
{
    static constexpr auto name = "FunctionSkeleton";

    static void data(auto& cls)
    {
        using V = urd::data<FunctionSkeleton>;
        cls.def(nb::init<std::string, urd::flat_vector<urd::index<Parameter>>, urd::index<Type>>(), "name"_a, "parameters"_a, "type"_a)
            .def(urd::python::init_from_views<const std::string&, const urd::Store<urd::sequence<ParameterView>>&, TypeView>(), "name"_a, "parameters"_a, "type"_a)
            .def_rw("name", &V::name)
            .def_rw("parameters", &V::parameters)
            .def_rw("type", &V::type);
    }

    static void view(auto& cls)
    {
        using V = FunctionSkeletonView;
        cls.def("get_name", &V::get_name).def("get_parameters", &V::get_parameters).def("get_arity", &V::get_arity).def("get_type", &V::get_type);
    }
};

template<>
struct Symbol<Atom>
{
    static constexpr auto name = "Atom";

    static void data(auto& cls)
    {
        using V = urd::data<Atom>;
        cls.def(nb::init<urd::index<Predicate>, TermList>(), "predicate"_a, "terms"_a = TermList {})
            .def(urd::python::init_from_views<PredicateView, const urd::Store<urd::sequence<TermView>>&>(), "predicate"_a, "terms"_a = urd::Store<urd::sequence<TermView>> {})
            .def_rw("predicate", &V::predicate)
            .def_rw("terms", &V::terms);
    }

    static void view(auto& cls)
    {
        using V = AtomView;
        cls.def("get_predicate", &V::get_predicate).def("get_terms", &V::get_terms);
    }
};

template<>
struct Symbol<Literal>
{
    static constexpr auto name = "Literal";

    static void data(auto& cls)
    {
        using V = urd::data<Literal>;
        cls.def(nb::init<urd::index<Atom>, bool>(), "atom"_a, "polarity"_a = true)
            .def(urd::python::init_from_views<AtomView, bool>(), "atom"_a, "polarity"_a = true)
            .def_rw("atom", &V::atom)
            .def_rw("polarity", &V::m_polarity);
    }

    static void view(auto& cls)
    {
        using V = LiteralView;
        cls.def("get_atom", &V::get_atom).def("get_polarity", &V::get_polarity);
    }
};

template<>
struct Symbol<FunctionExpression<NumberTag>>
{
    static constexpr auto name = "FunctionExpressionNumber";

    static void data(auto& cls)
    {
        using V = urd::data<FunctionExpression<NumberTag>>;
        cls.def(nb::init<double>(), "value"_a).def_rw("value", &V::value);
    }

    static void view(auto& cls)
    {
        using V = EntityView<FunctionExpression<NumberTag>>;
        cls.def("get_value", &V::get_value);
    }
};

template<>
struct Symbol<FunctionTerm>
{
    static constexpr auto name = "FunctionTerm";

    static void data(auto& cls)
    {
        using V = urd::data<FunctionTerm>;
        cls.def(nb::init<urd::index<FunctionSkeleton>, TermList>(),
                 "function"_a,
                 "terms"_a = TermList {})
            .def(urd::python::init_from_views<FunctionSkeletonView, const urd::Store<urd::sequence<TermView>>&>(),
                 "function"_a,
                 "terms"_a = urd::Store<urd::sequence<TermView>> {})
            .def_rw("function", &V::function)
            .def_rw("terms", &V::terms);
    }

    static void view(auto& cls)
    {
        using V = FunctionTermView;
        cls.def("get_function", &V::get_function).def("get_terms", &V::get_terms);
    }
};

template<>
struct Symbol<FunctionExpression<UnaryTag>>
{
    static constexpr auto name = "UnaryFunctionExpression";

    static void data(auto& cls)
    {
        using V = urd::data<FunctionExpression<UnaryTag>>;
        cls.def(nb::init<UnaryArithmeticOperator, FunctionExpressionVariant>(), "operator"_a, "expression"_a)
            .def(urd::python::init_from_views<UnaryArithmeticOperator, FunctionExpressionView>(), "operator"_a, "expression"_a)
            .def_rw("operator", &V::op)
            .def_rw("expression", &V::expression);
    }

    static void view(auto& cls)
    {
        using V = EntityView<FunctionExpression<UnaryTag>>;
        cls.def("get_operator", &V::get_operator).def("get_expression", &V::get_expression);
    }
};

template<>
struct Symbol<FunctionExpression<BinaryTag>>
{
    static constexpr auto name = "BinaryFunctionExpression";

    static void data(auto& cls)
    {
        using V = urd::data<FunctionExpression<BinaryTag>>;
        cls.def(nb::init<BinaryArithmeticOperator, FunctionExpressionVariant, FunctionExpressionVariant>(),
                 "operator"_a,
                 "left"_a,
                 "right"_a)
            .def(urd::python::init_from_views<BinaryArithmeticOperator, FunctionExpressionView, FunctionExpressionView>(),
                 "operator"_a,
                 "left"_a,
                 "right"_a)
            .def_rw("operator", &V::op)
            .def_rw("left", &V::left)
            .def_rw("right", &V::right);
    }

    static void view(auto& cls)
    {
        using V = EntityView<FunctionExpression<BinaryTag>>;
        cls.def("get_operator", &V::get_operator).def("get_left", &V::get_left).def("get_right", &V::get_right);
    }
};

template<>
struct Symbol<FunctionExpression<MultiTag>>
{
    static constexpr auto name = "MultiFunctionExpression";

    static void data(auto& cls)
    {
        using V = urd::data<FunctionExpression<MultiTag>>;
        cls.def(nb::init<MultiArithmeticOperator, FunctionExpressionList>(), "operator"_a, "args"_a)
            .def(urd::python::init_from_views<MultiArithmeticOperator, const urd::Store<urd::sequence<FunctionExpressionView>>&>(),
                 "operator"_a,
                 "args"_a = urd::Store<urd::sequence<FunctionExpressionView>> {})
            .def_rw("operator", &V::op)
            .def_rw("args", &V::args);
    }

    static void view(auto& cls)
    {
        using V = EntityView<FunctionExpression<MultiTag>>;
        const auto attach_owner = nb::cpp_function([](nb::object child, const nb::object&) { return child; }, nb::keep_alive<0, 2>());
        cls.def("get_operator", &V::get_operator)
            .def("get_args",
                 [attach_owner](const V& self)
                 {
                     using Arg = std::ranges::range_value_t<decltype(self.get_args())>;
                     auto result = nb::typed<nb::list, Arg>();
                     const auto parent = nb::find(self);
                     for (const auto expression : self.get_args())
                     {
                         result.append(attach_owner(nb::cast(expression), parent));
                     }
                     return result;
                 });
    }
};

template<>
struct Symbol<Condition<LiteralTag>>
{
    static constexpr auto name = "ConditionLiteral";

    static void data(auto& cls)
    {
        using V = urd::data<Condition<LiteralTag>>;
        cls.def(nb::init<urd::index<Literal>>(), "literal"_a)
            .def(urd::python::init_from_views<LiteralView>(), "literal"_a)
            .def_rw("literal", &V::literal);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Condition<LiteralTag>>;
        cls.def("get_literal", &V::get_literal);
    }
};

template<>
struct Symbol<Condition<AndTag>>
{
    static constexpr auto name = "ConditionAnd";

    static void data(auto& cls)
    {
        using V = urd::data<Condition<AndTag>>;
        cls.def(nb::init<ConditionList>(), "conditions"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<ConditionView>>&>(), "conditions"_a)
            .def_rw("conditions", &V::conditions);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Condition<AndTag>>;
        cls.def("get_conditions", &V::get_conditions);
    }
};

template<>
struct Symbol<Condition<OrTag>>
{
    static constexpr auto name = "ConditionOr";

    static void data(auto& cls)
    {
        using V = urd::data<Condition<OrTag>>;
        cls.def(nb::init<ConditionList>(), "conditions"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<ConditionView>>&>(), "conditions"_a)
            .def_rw("conditions", &V::conditions);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Condition<OrTag>>;
        cls.def("get_conditions", &V::get_conditions);
    }
};

template<>
struct Symbol<Condition<NotTag>>
{
    static constexpr auto name = "ConditionNot";

    static void data(auto& cls)
    {
        using V = urd::data<Condition<NotTag>>;
        cls.def(nb::init<ConditionVariant>(), "condition"_a)
            .def(urd::python::init_from_views<ConditionView>(), "condition"_a)
            .def_rw("condition", &V::condition);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Condition<NotTag>>;
        cls.def("get_condition", &V::get_condition);
    }
};

template<>
struct Symbol<Condition<ImplyTag>>
{
    static constexpr auto name = "ConditionImply";

    static void data(auto& cls)
    {
        using V = urd::data<Condition<ImplyTag>>;
        cls.def(nb::init<ConditionVariant, ConditionVariant>(), "left"_a, "right"_a)
            .def(urd::python::init_from_views<ConditionView, ConditionView>(), "left"_a, "right"_a)
            .def_rw("left", &V::left)
            .def_rw("right", &V::right);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Condition<ImplyTag>>;
        cls.def("get_left", &V::get_left).def("get_right", &V::get_right);
    }
};

template<>
struct Symbol<Condition<ExistsTag>>
{
    static constexpr auto name = "ConditionExists";

    static void data(auto& cls)
    {
        using V = urd::data<Condition<ExistsTag>>;
        cls.def(nb::init<urd::flat_vector<urd::index<Parameter>>, ConditionVariant>(), "parameters"_a, "condition"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<ParameterView>>&, ConditionView>(), "parameters"_a, "condition"_a)
            .def_rw("parameters", &V::parameters)
            .def_rw("condition", &V::condition);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Condition<ExistsTag>>;
        cls.def("get_parameters", &V::get_parameters).def("get_condition", &V::get_condition);
    }
};

template<>
struct Symbol<Condition<ForallTag>>
{
    static constexpr auto name = "ConditionForall";

    static void data(auto& cls)
    {
        using V = urd::data<Condition<ForallTag>>;
        cls.def(nb::init<urd::flat_vector<urd::index<Parameter>>, ConditionVariant>(), "parameters"_a, "condition"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<ParameterView>>&, ConditionView>(), "parameters"_a, "condition"_a)
            .def_rw("parameters", &V::parameters)
            .def_rw("condition", &V::condition);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Condition<ForallTag>>;
        cls.def("get_parameters", &V::get_parameters).def("get_condition", &V::get_condition);
    }
};

template<>
struct Symbol<Condition<NumericConstraintTag>>
{
    static constexpr auto name = "ConditionNumericConstraint";

    static void data(auto& cls)
    {
        using V = urd::data<Condition<NumericConstraintTag>>;
        cls.def(nb::init<BinaryComparator, FunctionExpressionVariant, FunctionExpressionVariant>(),
                 "comparator"_a,
                 "left"_a,
                 "right"_a)
            .def(urd::python::init_from_views<BinaryComparator, FunctionExpressionView, FunctionExpressionView>(),
                 "comparator"_a,
                 "left"_a,
                 "right"_a)
            .def_rw("comparator", &V::comparator)
            .def_rw("left", &V::left)
            .def_rw("right", &V::right);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Condition<NumericConstraintTag>>;
        cls.def("get_comparator", &V::get_comparator).def("get_left", &V::get_left).def("get_right", &V::get_right);
    }
};

template<>
struct Symbol<Effect<LiteralTag>>
{
    static constexpr auto name = "EffectLiteral";

    static void data(auto& cls)
    {
        using V = urd::data<Effect<LiteralTag>>;
        cls.def(nb::init<urd::index<Literal>>(), "literal"_a)
            .def(urd::python::init_from_views<LiteralView>(), "literal"_a)
            .def_rw("literal", &V::literal);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Effect<LiteralTag>>;
        cls.def("get_literal", &V::get_literal);
    }
};

template<>
struct Symbol<Effect<AndTag>>
{
    static constexpr auto name = "EffectAnd";

    static void data(auto& cls)
    {
        using V = urd::data<Effect<AndTag>>;
        cls.def(nb::init<EffectList>(), "effects"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<EffectView>>&>(), "effects"_a)
            .def_rw("effects", &V::effects);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Effect<AndTag>>;
        cls.def("get_effects", &V::get_effects);
    }
};

template<>
struct Symbol<Effect<NumericTag>>
{
    static constexpr auto name = "EffectNumeric";

    static void data(auto& cls)
    {
        using V = urd::data<Effect<NumericTag>>;
        cls.def(nb::init<NumericEffectOperator, urd::index<FunctionTerm>, FunctionExpressionVariant>(),
                 "operator"_a,
                 "function"_a,
                 "expression"_a)
            .def(urd::python::init_from_views<NumericEffectOperator, FunctionTermView, FunctionExpressionView>(),
                 "operator"_a,
                 "function"_a,
                 "expression"_a)
            .def_rw("operator", &V::op)
            .def_rw("function", &V::function)
            .def_rw("expression", &V::expression);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Effect<NumericTag>>;
        cls.def("get_operator", &V::get_operator).def("get_function", &V::get_function).def("get_expression", &V::get_expression);
    }
};

template<>
struct Symbol<Effect<ForallTag>>
{
    static constexpr auto name = "EffectForall";

    static void data(auto& cls)
    {
        using V = urd::data<Effect<ForallTag>>;
        cls.def(nb::init<urd::flat_vector<urd::index<Parameter>>, EffectVariant>(), "parameters"_a, "effect"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<ParameterView>>&, EffectView>(), "parameters"_a, "effect"_a)
            .def_rw("parameters", &V::parameters)
            .def_rw("effect", &V::effect);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Effect<ForallTag>>;
        cls.def("get_parameters", &V::get_parameters).def("get_effect", &V::get_effect);
    }
};

template<>
struct Symbol<Effect<WhenTag>>
{
    static constexpr auto name = "EffectWhen";

    static void data(auto& cls)
    {
        using V = urd::data<Effect<WhenTag>>;
        cls.def(nb::init<ConditionVariant, EffectVariant>(), "condition"_a, "effect"_a)
            .def(urd::python::init_from_views<ConditionView, EffectView>(), "condition"_a, "effect"_a)
            .def_rw("condition", &V::condition)
            .def_rw("effect", &V::effect);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Effect<WhenTag>>;
        cls.def("get_condition", &V::get_condition).def("get_effect", &V::get_effect);
    }
};

template<>
struct Symbol<Effect<OneOfTag>>
{
    static constexpr auto name = "EffectOneOf";

    static void data(auto& cls)
    {
        using V = urd::data<Effect<OneOfTag>>;
        cls.def(nb::init<EffectList>(), "effects"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<EffectView>>&>(), "effects"_a)
            .def_rw("effects", &V::effects);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Effect<OneOfTag>>;
        cls.def("get_effects", &V::get_effects);
    }
};

template<>
struct Symbol<EffectProbabilisticAlternative>
{
    static constexpr auto name = "EffectProbabilisticAlternative";

    static void data(auto& cls)
    {
        using V = urd::data<EffectProbabilisticAlternative>;
        cls.def(nb::init<double, EffectVariant>(), "probability"_a, "effect"_a)
            .def(urd::python::init_from_views<double, EffectView>(), "probability"_a, "effect"_a)
            .def_rw("probability", &V::probability)
            .def_rw("effect", &V::effect);
    }

    static void view(auto& cls)
    {
        using V = EffectProbabilisticAlternativeView;
        cls.def("get_probability", &V::get_probability).def("get_effect", &V::get_effect);
    }
};

template<>
struct Symbol<Effect<ProbabilisticTag>>
{
    static constexpr auto name = "EffectProbabilistic";

    static void data(auto& cls)
    {
        using V = urd::data<Effect<ProbabilisticTag>>;
        cls.def(nb::init<urd::flat_vector<urd::index<EffectProbabilisticAlternative>>>(), "alternatives"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<EffectProbabilisticAlternativeView>>&>(), "alternatives"_a)
            .def_rw("alternatives", &V::alternatives);
    }

    static void view(auto& cls)
    {
        using V = EntityView<Effect<ProbabilisticTag>>;
        cls.def("get_alternatives", &V::get_alternatives);
    }
};

template<>
struct Symbol<Action>
{
    static constexpr auto name = "Action";

    static void data(auto& cls)
    {
        using V = urd::data<Action>;
        cls.def(nb::init<std::string,
                          urd::flat_vector<urd::index<Parameter>>,
                          urd::flat_optional<ConditionVariant>,
                          urd::flat_optional<EffectVariant>>(),
                 "name"_a,
                 "parameters"_a = urd::flat_vector<urd::index<Parameter>> {},
                 "precondition"_a.none() = urd::flat_optional<ConditionVariant> {},
                 "effect"_a.none() = urd::flat_optional<EffectVariant> {})
            .def(nb::init<std::string,
                          std::string,
                          urd::flat_vector<urd::index<Parameter>>,
                          ygg::uint_t,
                          urd::flat_optional<ConditionVariant>,
                          urd::flat_optional<EffectVariant>>(),
                 "name"_a,
                 "original_name"_a,
                 "parameters"_a,
                 "original_arity"_a,
                 "precondition"_a.none() = urd::flat_optional<ConditionVariant> {},
                 "effect"_a.none() = urd::flat_optional<EffectVariant> {})
            .def(urd::python::init_from_views<const std::string&,
                          const urd::Store<urd::sequence<ParameterView>>&,
                          const std::optional<ConditionView>&,
                          const std::optional<EffectView>&>(),
                 "name"_a,
                 "parameters"_a = urd::Store<urd::sequence<ParameterView>> {},
                 "precondition"_a = std::optional<ConditionView> {},
                 "effect"_a = std::optional<EffectView> {})
            .def(urd::python::init_from_views<const std::string&,
                          const std::string&,
                          const urd::Store<urd::sequence<ParameterView>>&,
                          ygg::uint_t,
                          const std::optional<ConditionView>&,
                          const std::optional<EffectView>&>(),
                 "name"_a,
                 "original_name"_a,
                 "parameters"_a,
                 "original_arity"_a,
                 "precondition"_a = std::optional<ConditionView> {},
                 "effect"_a = std::optional<EffectView> {})
            .def_rw("name", &V::name)
            .def_rw("original_name", &V::original_name)
            .def_rw("parameters", &V::parameters)
            .def_rw("original_arity", &V::original_arity)
            .def_rw("precondition", &V::precondition)
            .def_rw("effect", &V::effect);
    }

    static void view(auto& cls)
    {
        using V = ActionView;
        cls.def("get_name", &V::get_name)
            .def("get_original_name", &V::get_original_name)
            .def("get_parameters", &V::get_parameters)
            .def("get_arity", &V::get_arity)
            .def("get_original_arity", &V::get_original_arity)
            .def("get_precondition", &V::get_precondition)
            .def("get_effect", &V::get_effect);
    }
};

template<>
struct Symbol<Axiom>
{
    static constexpr auto name = "Axiom";

    static void data(auto& cls)
    {
        using V = urd::data<Axiom>;
        cls.def(nb::init<urd::flat_vector<urd::index<Parameter>>, urd::index<Literal>, ConditionVariant>(),
                 "parameters"_a,
                 "head"_a,
                 "condition"_a)
            .def(nb::init<urd::flat_vector<urd::index<Parameter>>, ygg::uint_t, urd::index<Literal>, ConditionVariant>(),
                 "parameters"_a,
                 "original_arity"_a,
                 "head"_a,
                 "condition"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<ParameterView>>&, LiteralView, ConditionView>(),
                 "parameters"_a,
                 "head"_a,
                 "condition"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<ParameterView>>&, ygg::uint_t, LiteralView, ConditionView>(),
                 "parameters"_a,
                 "original_arity"_a,
                 "head"_a,
                 "condition"_a)
            .def_rw("parameters", &V::parameters)
            .def_rw("original_arity", &V::original_arity)
            .def_rw("head", &V::head)
            .def_rw("condition", &V::condition);
    }

    static void view(auto& cls)
    {
        using V = AxiomView;
        cls.def("get_parameters", &V::get_parameters)
            .def("get_arity", &V::get_arity)
            .def("get_original_arity", &V::get_original_arity)
            .def("get_head", &V::get_head)
            .def("get_condition", &V::get_condition);
    }
};

template<>
struct Symbol<Metric>
{
    static constexpr auto name = "Metric";

    static void data(auto& cls)
    {
        using V = urd::data<Metric>;
        cls.def(nb::init<OptimizationDirection, FunctionExpressionVariant>(), "optimization_direction"_a, "expression"_a)
            .def(urd::python::init_from_views<OptimizationDirection, FunctionExpressionView>(), "optimization_direction"_a, "expression"_a)
            .def_rw("optimization_direction", &V::optimization_direction)
            .def_rw("expression", &V::expression);
    }

    static void view(auto& cls)
    {
        using V = MetricView;
        cls.def("get_optimization_direction", &V::get_optimization_direction).def("get_expression", &V::get_expression);
    }
};

template<>
struct Symbol<InitialFunctionValue>
{
    static constexpr auto name = "InitialFunctionValue";

    static void data(auto& cls)
    {
        using V = urd::data<InitialFunctionValue>;
        cls.def(nb::init<urd::index<FunctionTerm>, FunctionExpressionVariant>(), "function"_a, "value"_a)
            .def(urd::python::init_from_views<FunctionTermView, FunctionExpressionView>(), "function"_a, "value"_a)
            .def_rw("function", &V::function)
            .def_rw("value", &V::value);
    }

    static void view(auto& cls)
    {
        using V = InitialFunctionValueView;
        cls.def("get_function", &V::get_function).def("get_value", &V::get_value);
    }
};

template<>
struct Symbol<Domain>
{
    static constexpr auto name = "Domain";

    static void data(auto& cls)
    {
        using V = urd::data<Domain>;
        cls.def(nb::init<std::string,
                          urd::flat_vector<urd::index<Requirement>>,
                          urd::flat_vector<urd::index<Type>>,
                          urd::flat_vector<urd::index<Object>>,
                          urd::flat_vector<urd::index<Predicate>>,
                          urd::flat_vector<urd::index<FunctionSkeleton>>,
                          urd::flat_vector<urd::index<Action>>,
                          urd::flat_vector<urd::index<Axiom>>>(),
                 "name"_a,
                 "requirements"_a = urd::flat_vector<urd::index<Requirement>> {},
                 "types"_a = urd::flat_vector<urd::index<Type>> {},
                 "constants"_a = urd::flat_vector<urd::index<Object>> {},
                 "predicates"_a = urd::flat_vector<urd::index<Predicate>> {},
                 "functions"_a = urd::flat_vector<urd::index<FunctionSkeleton>> {},
                 "actions"_a = urd::flat_vector<urd::index<Action>> {},
                 "axioms"_a = urd::flat_vector<urd::index<Axiom>> {})
            .def(urd::python::init_from_views<const std::string&,
                          const urd::Store<urd::sequence<RequirementView>>&,
                          const urd::Store<urd::sequence<TypeView>>&,
                          const urd::Store<urd::sequence<ObjectView>>&,
                          const urd::Store<urd::sequence<PredicateView>>&,
                          const urd::Store<urd::sequence<FunctionSkeletonView>>&,
                          const urd::Store<urd::sequence<ActionView>>&,
                          const urd::Store<urd::sequence<AxiomView>>&>(),
                 "name"_a,
                 "requirements"_a = urd::Store<urd::sequence<RequirementView>> {},
                 "types"_a = urd::Store<urd::sequence<TypeView>> {},
                 "constants"_a = urd::Store<urd::sequence<ObjectView>> {},
                 "predicates"_a = urd::Store<urd::sequence<PredicateView>> {},
                 "functions"_a = urd::Store<urd::sequence<FunctionSkeletonView>> {},
                 "actions"_a = urd::Store<urd::sequence<ActionView>> {},
                 "axioms"_a = urd::Store<urd::sequence<AxiomView>> {})
            .def_rw("name", &V::name)
            .def_rw("requirements", &V::requirements)
            .def_rw("types", &V::types)
            .def_rw("constants", &V::constants)
            .def_rw("predicates", &V::predicates)
            .def_rw("functions", &V::functions)
            .def_rw("actions", &V::actions)
            .def_rw("axioms", &V::axioms);
    }

    static void view(auto& cls)
    {
        using V = DomainView;
        cls.def("get_name", &V::get_name)
            .def("get_requirements", &V::get_requirements)
            .def("get_types", &V::get_types)
            .def("get_constants", &V::get_constants)
            .def("get_predicates", &V::get_predicates)
            .def("get_functions", &V::get_functions)
            .def("get_actions", &V::get_actions)
            .def("get_axioms", &V::get_axioms);
    }
};

template<>
struct Symbol<Task>
{
    static constexpr auto name = "Task";

    static void data(auto& cls)
    {
        using V = urd::data<Task>;
        cls.def(nb::init<std::string,
                          urd::index<Domain>,
                          urd::flat_vector<urd::index<Requirement>>,
                          urd::flat_vector<urd::index<Type>>,
                          urd::flat_vector<urd::index<Predicate>>,
                          urd::flat_vector<urd::index<Object>>,
                          urd::flat_vector<urd::index<Literal>>,
                          urd::flat_vector<urd::index<InitialFunctionValue>>,
                          urd::flat_optional<ConditionVariant>,
                          urd::flat_optional<urd::index<Metric>>,
                          urd::flat_vector<urd::index<Axiom>>>(),
                 "name"_a,
                 "domain"_a,
                 "requirements"_a = urd::flat_vector<urd::index<Requirement>> {},
                 "types"_a = urd::flat_vector<urd::index<Type>> {},
                 "predicates"_a = urd::flat_vector<urd::index<Predicate>> {},
                 "objects"_a = urd::flat_vector<urd::index<Object>> {},
                 "initial_literals"_a = urd::flat_vector<urd::index<Literal>> {},
                 "initial_function_values"_a = urd::flat_vector<urd::index<InitialFunctionValue>> {},
                 "goal"_a.none() = urd::flat_optional<ConditionVariant> {},
                 "metric"_a.none() = urd::flat_optional<urd::index<Metric>> {},
                 "axioms"_a = urd::flat_vector<urd::index<Axiom>> {})
            .def(urd::python::init_from_views<const std::string&,
                          DomainView,
                          const urd::Store<urd::sequence<RequirementView>>&,
                          const urd::Store<urd::sequence<TypeView>>&,
                          const urd::Store<urd::sequence<PredicateView>>&,
                          const urd::Store<urd::sequence<ObjectView>>&,
                          const urd::Store<urd::sequence<LiteralView>>&,
                          const urd::Store<urd::sequence<InitialFunctionValueView>>&,
                          const std::optional<ConditionView>&,
                          const std::optional<MetricView>&,
                          const urd::Store<urd::sequence<AxiomView>>&>(),
                 "name"_a,
                 "domain"_a,
                 "requirements"_a = urd::Store<urd::sequence<RequirementView>> {},
                 "types"_a = urd::Store<urd::sequence<TypeView>> {},
                 "predicates"_a = urd::Store<urd::sequence<PredicateView>> {},
                 "objects"_a = urd::Store<urd::sequence<ObjectView>> {},
                 "initial_literals"_a = urd::Store<urd::sequence<LiteralView>> {},
                 "initial_function_values"_a = urd::Store<urd::sequence<InitialFunctionValueView>> {},
                 "goal"_a = std::optional<ConditionView> {},
                 "metric"_a = std::optional<MetricView> {},
                 "axioms"_a = urd::Store<urd::sequence<AxiomView>> {})
            .def_rw("name", &V::name)
            .def_rw("domain", &V::domain)
            .def_rw("requirements", &V::requirements)
            .def_rw("types", &V::types)
            .def_rw("predicates", &V::predicates)
            .def_rw("objects", &V::objects)
            .def_rw("initial_literals", &V::initial_literals)
            .def_rw("initial_function_values", &V::initial_function_values)
            .def_rw("goal", &V::goal)
            .def_rw("metric", &V::metric)
            .def_rw("axioms", &V::axioms);
    }

    static void view(auto& cls)
    {
        using V = TaskView;
        cls.def("get_name", &V::get_name)
            .def("get_domain", &V::get_domain)
            .def("get_requirements", &V::get_requirements)
            .def("get_types", &V::get_types)
            .def("get_predicates", &V::get_predicates)
            .def("get_objects", &V::get_objects)
            .def("get_initial_literals", &V::get_initial_literals)
            .def("get_initial_function_values", &V::get_initial_function_values)
            .def("get_goal", &V::get_goal)
            .def("get_metric", &V::get_metric)
            .def("get_axioms", &V::get_axioms);
    }
};

/// The index, data and view classes of a symbol type, and the repository's insert for its data.
template<typename T>
void bind_symbol(nb::module_& m, RepositoryBinding& repository)
{
    const auto name = std::string(Symbol<T>::name);
    urd::python::bind_index<urd::index<T>>(m, name + "Index");

    auto data = urd::python::bind_data<urd::data<T>>(m, (name + "Data").c_str());
    Symbol<T>::data(data);

    auto view = nb::class_<EntityView<T>>(m, name.c_str());
    view.def("index", &EntityView<T>::index);
    Symbol<T>::view(view);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);

    bind_insert<T>(repository);
}

}  // namespace

void bind_formalism(nb::module_& m)
{
    bind_formalism_enums(m);

    auto repository = RepositoryBinding(m, "Repository", "Owns interned formalism objects created from builder data.");
    repository.def(nb::init<>(), "Create a repository that owns interned formalism objects.");
    boost::mp11::mp_for_each<boost::mp11::mp_transform<boost::mp11::mp_identity, SymbolRepositoryTypes>>(
        [&](auto type) { bind_symbol<typename decltype(type)::type>(m, repository); });
}

}  // namespace loki::formalism
