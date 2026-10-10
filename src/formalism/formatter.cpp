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

#include "loki/formalism/formatter.hpp"

#include "loki/formalism/repository.hpp"
#include "loki/formalism/views.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/formatting/cista_formatters.hpp>
#include <yggdrasil/io/iostream.hpp>

namespace loki::formalism::format
{

namespace detail
{

inline bool is_builtin_type(TypeView type)
{
    const auto name = std::string_view(type.get_name().data(), type.get_name().size());
    return name == "object" || name == "number";
}

// Serializes a range as " e1 e2 ..." with a leading space per element; an empty range yields "".
template<typename Range>
inline std::string spaced(const Range& range)
{
    auto result = std::string {};
    for (auto element : range)
        fmt::format_to(std::back_inserter(result), " {}", to_string(element));
    return result;
}

// Renders " - type" or " - (either t1 t2 ...)"; multiple types require an either wrapper to reparse.
template<typename Types>
inline std::string type_annotation(const Types& types)
{
    if (types.size() == 1)
        return fmt::format(" - {}", to_string(types[0]));
    return fmt::format(" - (either{})", spaced(types));
}

}  // namespace detail

std::string to_string(RequirementView value) { return std::string(loki::formalism::to_string(value.get_kind())); }

std::string to_string(TypeView value) { return fmt::format("{}", value.get_name()); }

std::string to_string(ObjectView value) { return fmt::format("{}", value.get_name()); }

std::string to_string(VariableView value) { return fmt::format("{}", value.get_name()); }

std::string to_string(ParameterView value)
{
    return fmt::format("{}{}", to_string(value.get_variable()), value.get_types().empty() ? "" : detail::type_annotation(value.get_types()));
}

std::string to_string(PredicateView value) { return fmt::format("({}{})", value.get_name(), detail::spaced(value.get_parameters())); }

std::string to_string(FunctionSkeletonView value)
{
    return fmt::format("({}{}) - {}", value.get_name(), detail::spaced(value.get_parameters()), to_string(value.get_type()));
}

std::string to_string(TermView value)
{
    auto result = std::string {};
    visit([&](const auto& node) { result = to_string(node); }, value.get_variant());
    return result;
}

std::string to_string(AtomView value) { return fmt::format("({}{})", value.get_predicate().get_name(), detail::spaced(value.get_terms())); }

std::string to_string(LiteralView value)
{
    if (value.get_polarity())
        return to_string(value.get_atom());
    return fmt::format("(not {})", to_string(value.get_atom()));
}

std::string to_string(EntityView<FunctionExpression<NumberTag>> value) { return fmt::format("{}", value.get_value()); }

std::string to_string(FunctionTermView value) { return fmt::format("({}{})", value.get_function().get_name(), detail::spaced(value.get_terms())); }

std::string to_string(EntityView<FunctionExpression<UnaryTag>> value) { return fmt::format("(- {})", to_string(value.get_expression())); }

std::string to_string(EntityView<FunctionExpression<BinaryTag>> value)
{
    return fmt::format("({} {} {})", loki::formalism::to_string(value.get_data().op), to_string(value.get_left()), to_string(value.get_right()));
}

std::string to_string(EntityView<FunctionExpression<MultiTag>> value)
{
    return fmt::format("({}{})", loki::formalism::to_string(value.get_operator()), detail::spaced(value.get_args()));
}

std::string to_string(FunctionExpressionView value)
{
    auto result = std::string {};
    visit([&](const auto& node) { result = to_string(node); }, value.get_variant());
    return result;
}

std::string to_string(EntityView<Condition<LiteralTag>> value) { return to_string(value.get_literal()); }

std::string to_string(EntityView<Condition<AndTag>> value) { return fmt::format("(and{})", detail::spaced(value.get_conditions())); }

std::string to_string(EntityView<Condition<OrTag>> value) { return fmt::format("(or{})", detail::spaced(value.get_conditions())); }

std::string to_string(EntityView<Condition<NotTag>> value) { return fmt::format("(not {})", to_string(value.get_condition())); }

std::string to_string(EntityView<Condition<ImplyTag>> value) { return fmt::format("(imply {} {})", to_string(value.get_left()), to_string(value.get_right())); }

namespace detail
{

// Materializes element strings so fmt::join never needs the guarded view formatters.
template<typename Range>
inline std::vector<std::string> to_strings(const Range& range)
{
    auto result = std::vector<std::string> {};
    for (auto element : range)
        result.push_back(to_string(element));
    return result;
}

}  // namespace detail

std::string to_string(EntityView<Condition<ExistsTag>> value)
{
    return fmt::format("(exists ({}) {})", fmt::join(detail::to_strings(value.get_parameters()), " "), to_string(value.get_condition()));
}

std::string to_string(EntityView<Condition<ForallTag>> value)
{
    return fmt::format("(forall ({}) {})", fmt::join(detail::to_strings(value.get_parameters()), " "), to_string(value.get_condition()));
}

std::string to_string(EntityView<Condition<NumericConstraintTag>> value)
{
    return fmt::format("({} {} {})", loki::formalism::to_string(value.get_data().comparator), to_string(value.get_left()), to_string(value.get_right()));
}

std::string to_string(ConditionView value)
{
    auto result = std::string {};
    visit([&](const auto& node) { result = to_string(node); }, value.get_variant());
    return result;
}

std::string to_string(EntityView<Effect<LiteralTag>> value) { return to_string(value.get_literal()); }

std::string to_string(EntityView<Effect<AndTag>> value) { return fmt::format("(and{})", detail::spaced(value.get_effects())); }

std::string to_string(EntityView<Effect<NumericTag>> value)
{
    return fmt::format("({} {} {})", loki::formalism::to_string(value.get_data().op), to_string(value.get_function()), to_string(value.get_expression()));
}

std::string to_string(EntityView<Effect<ForallTag>> value)
{
    return fmt::format("(forall ({}) {})", fmt::join(detail::to_strings(value.get_parameters()), " "), to_string(value.get_effect()));
}

std::string to_string(EntityView<Effect<WhenTag>> value) { return fmt::format("(when {} {})", to_string(value.get_condition()), to_string(value.get_effect())); }

std::string to_string(EntityView<Effect<OneOfTag>> value) { return fmt::format("(oneof{})", detail::spaced(value.get_effects())); }

std::string to_string(EffectProbabilisticAlternativeView value) { return fmt::format("{} {}", value.get_data().probability, to_string(value.get_effect())); }

std::string to_string(EntityView<Effect<ProbabilisticTag>> value) { return fmt::format("(probabilistic{})", detail::spaced(value.get_alternatives())); }

std::string to_string(EffectView value)
{
    auto result = std::string {};
    visit([&](const auto& node) { result = to_string(node); }, value.get_variant());
    return result;
}

std::string to_string(ActionView value)
{
    return fmt::format("(:action {} :parameters ({}){}{})",
                       value.get_name(),
                       fmt::join(detail::to_strings(value.get_parameters()), " "),
                       value.get_precondition() ? fmt::format(" :precondition {}", to_string(value.get_precondition().value())) : "",
                       value.get_effect() ? fmt::format(" :effect {}", to_string(value.get_effect().value())) : "");
}

std::string to_string(AxiomView value)
{
    auto head_variables = ygg::UnorderedSet<VariableView> {};
    for (auto term : value.get_head().get_atom().get_terms())
    {
        visit(
            [&](const auto& node)
            {
                using T = std::remove_cvref_t<decltype(node)>;
                if constexpr (std::same_as<T, VariableView>)
                    head_variables.insert(node);
            },
            term.get_variant());
    }

    auto existential_parameters = std::vector<ParameterView> {};
    for (auto parameter : value.get_parameters())
    {
        if (!head_variables.contains(parameter.get_variable()))
            existential_parameters.push_back(parameter);
    }

    auto result = fmt::format("(:derived {} ", to_string(value.get_head()));
    if (!existential_parameters.empty())
        return result + fmt::format("(exists ({}) {}))", fmt::join(detail::to_strings(existential_parameters), " "), to_string(value.get_condition()));
    return result + fmt::format("{})", to_string(value.get_condition()));
}

std::string to_string(MetricView value)
{
    return fmt::format("(:metric {} {})", formalism::to_string(value.get_optimization_direction()), to_string(value.get_expression()));
}

std::string to_string(InitialFunctionValueView value) { return fmt::format("(= {} {})", to_string(value.get_function()), to_string(value.get_value())); }

std::string to_string(DomainView value)
{
    auto os = std::stringstream {};
    os << fmt::format("(define (domain {})", value.get_name());
    {
        ygg::IndentScope section(os);
        if (!value.get_requirements().empty())
        {
            os << '\n' << ygg::print_indent << "(:requirements";
            for (auto requirement : value.get_requirements())
                os << ' ' << to_string(requirement);
            os << ')';
        }
        if (!value.get_types().empty())
        {
            auto wrote_header = false;
            for (auto type : value.get_types())
            {
                if (detail::is_builtin_type(type))
                    continue;
                if (!wrote_header)
                {
                    os << '\n' << ygg::print_indent << "(:types";
                    wrote_header = true;
                }
                os << ' ' << to_string(type);
                if (!type.get_bases().empty())
                    os << detail::type_annotation(type.get_bases());
            }
            if (wrote_header)
                os << ')';
        }
        if (!value.get_constants().empty())
        {
            os << '\n' << ygg::print_indent << "(:constants ";
            auto first = true;
            for (auto object : value.get_constants())
            {
                if (!first)
                    os << ' ';
                first = false;
                os << to_string(object);
                if (!object.get_types().empty())
                    os << detail::type_annotation(object.get_types());
            }
            os << ')';
        }
        if (!value.get_predicates().empty())
        {
            os << '\n' << ygg::print_indent << "(:predicates";
            {
                ygg::IndentScope predicates(os);
                for (auto predicate : value.get_predicates())
                    os << '\n' << ygg::print_indent << to_string(predicate);
            }
            os << ')';
        }
        if (!value.get_functions().empty())
        {
            os << '\n' << ygg::print_indent << "(:functions";
            {
                ygg::IndentScope functions(os);
                for (auto function : value.get_functions())
                    os << '\n' << ygg::print_indent << to_string(function);
            }
            os << ')';
        }
        for (auto axiom : value.get_axioms())
            os << '\n' << ygg::print_indent << to_string(axiom);
        for (auto action : value.get_actions())
            os << '\n' << ygg::print_indent << to_string(action);
    }
    os << "\n)";
    return os.str();
}

std::string to_string(TaskView value)
{
    auto os = std::stringstream {};
    os << fmt::format("(define (problem {})", value.get_name());
    {
        ygg::IndentScope section(os);
        os << '\n' << ygg::print_indent << fmt::format("(:domain {})", value.get_domain().get_name());
        if (!value.get_requirements().empty())
        {
            os << '\n' << ygg::print_indent << "(:requirements";
            for (auto requirement : value.get_requirements())
                os << ' ' << to_string(requirement);
            os << ')';
        }
        if (!value.get_objects().empty())
        {
            os << '\n' << ygg::print_indent << "(:objects ";
            auto first = true;
            for (auto object : value.get_objects())
            {
                if (!first)
                    os << ' ';
                first = false;
                os << to_string(object);
                if (!object.get_types().empty())
                    os << detail::type_annotation(object.get_types());
            }
            os << ')';
        }
        os << '\n' << ygg::print_indent << "(:init";
        for (auto literal : value.get_initial_literals())
            os << ' ' << to_string(literal);
        for (auto initial_value : value.get_initial_function_values())
            os << ' ' << to_string(initial_value);
        os << ')';
        if (value.get_goal())
            os << '\n' << ygg::print_indent << "(:goal " << to_string(value.get_goal().value()) << ')';
        if (value.get_metric())
            os << '\n' << ygg::print_indent << to_string(value.get_metric().value());
        for (auto axiom : value.get_axioms())
            os << '\n' << ygg::print_indent << to_string(axiom);
    }
    os << "\n)";
    return os.str();
}

}  // namespace loki::formalism::format
