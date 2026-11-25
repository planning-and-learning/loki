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

#ifndef LOKI_SEMANTIC_TRANSLATOR_COMPILE_TYPING_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_COMPILE_TYPING_TRANSLATOR_HPP_

#include "copy_translator_component.hpp"

#include <optional>
#include <string>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace loki::semantic::detail
{

template<typename Derived>
class CompileTypingTranslator : public CopyTranslatorComponent<Derived, CompileTypingTranslator<Derived>>
{
public:
    explicit CompileTypingTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, CompileTypingTranslator<Derived>>(context) {}

    [[nodiscard]] bool compiles_typing_now() const noexcept;
    urd::Store<urd::sequence<formalism::TypeView>> collect_type_hierarchy(formalism::TypeView type);
    void copy_type_hierarchy(formalism::EntityListView<formalism::Type> types,
                             urd::built_t<urd::flat_vector<urd::index<formalism::Type>>, formalism::Repository>& result);
    void copy_parameters_without_types(formalism::EntityListView<formalism::Parameter> parameters,
                                       urd::built_t<urd::flat_vector<urd::index<formalism::Parameter>>, formalism::Repository>& result);
    formalism::PredicateView type_predicate(formalism::TypeView type);
    formalism::LiteralView type_literal(formalism::TypeView type, formalism::TermView term);
    formalism::ConditionView type_condition(formalism::TypeView type, formalism::VariableView variable);
    void type_conditions_for_parameters(formalism::EntityListView<formalism::Parameter> parameters,
                                        urd::builder<formalism::Condition<formalism::AndTag>, formalism::Repository>& data);
    void prepend_type_conditions(std::optional<formalism::ConditionView>& condition, formalism::EntityListView<formalism::Parameter> parameters);
    void prepend_type_conditions(formalism::ConditionView& condition, formalism::EntityListView<formalism::Parameter> parameters);
    void compile_typing_to_domain(urd::builder<formalism::Domain, formalism::Repository>& data, formalism::DomainView domain);
    /// The type conditions of the parameters implying the condition, a conjunction: the head of a generated derived
    /// predicate loki-typed-*(free parameters) with an axiom per negated type condition and one for the condition, so that
    /// each axiom stays a conjunction. A universal constraint or preference holds vacuously outside its types. Without a
    /// condition, the parameters are outside their types: a disjunct that guards a universal formula over the trajectory.
    formalism::ConditionView guard_with_types(std::optional<formalism::ConditionView> condition, formalism::EntityListView<formalism::Parameter> parameters);
    /// A durative action's condition with the type conditions of its parameters added to its at start condition.
    std::optional<formalism::ConditionView> prepend_start_type_conditions(std::optional<formalism::ConditionView> condition,
                                                                          formalism::EntityListView<formalism::Parameter> parameters);
    void add_type_literals_for_object(urd::built_t<urd::flat_vector<urd::index<formalism::Literal>>, formalism::Repository>& literals,
                                      formalism::ObjectView object);
    void initialize_type_literals(urd::builder<formalism::Task, formalism::Repository>& data, formalism::TaskView task);
};

template<typename Derived>
bool CompileTypingTranslator<Derived>::compiles_typing_now() const noexcept
{ return this->copy_context().compile_typing && this->copy_context().phase == TranslationPhase::CompileTyping; }

template<typename Derived>
urd::Store<urd::sequence<formalism::TypeView>> CompileTypingTranslator<Derived>::collect_type_hierarchy(formalism::TypeView type)
{
    auto result = urd::Store<urd::sequence<formalism::TypeView>> {};
    auto seen = urd::Store<urd::set<formalism::TypeView>> {};
    auto visit = [&](auto&& self, formalism::TypeView current) -> void
    {
        if (!seen.insert(current).second)
            return;
        result.push_back(current);
        for (auto base : current.get_bases())
            self(self, base);
    };
    visit(visit, type);
    return result;
}

template<typename Derived>
void CompileTypingTranslator<Derived>::copy_type_hierarchy(formalism::EntityListView<formalism::Type> types,
                                                           urd::built_t<urd::flat_vector<urd::index<formalism::Type>>, formalism::Repository>& result)
{
    auto seen = urd::Store<urd::set<formalism::TypeView>> {};
    for (auto type : types)
    {
        for (auto collected : this->self().collect_type_hierarchy(type))
        {
            if (seen.insert(collected).second)
                urd::append(result, this->self().copy(collected));
        }
    }
}

template<typename Derived>
void CompileTypingTranslator<Derived>::copy_parameters_without_types(
    formalism::EntityListView<formalism::Parameter> parameters,
    urd::built_t<urd::flat_vector<urd::index<formalism::Parameter>>, formalism::Repository>& result)
{
    for (auto parameter : parameters)
    {
        const auto variable = this->self().copy(parameter.get_variable());
        auto data = formalism::checkout<formalism::Parameter>(this->copy_context().builder);
        urd::store(data->variable, variable);
        result.push_back(this->copy_context().storage->repository.insert(*data, this->copy_context().storage->builder).first.index());
    }
}

template<typename Derived>
formalism::PredicateView CompileTypingTranslator<Derived>::type_predicate(formalism::TypeView type)
{
    if (auto it = this->copy_context().type_predicates.find(type); it != this->copy_context().type_predicates.end())
        return it->second;

    auto parameter_data = formalism::checkout<formalism::Parameter>(this->copy_context().builder);
    // A type predicate tests any object for membership, so its argument has the root of the type's hierarchy (object, or number).
    if (!this->self().compiles_typing_now())
        for (auto ancestor : this->self().collect_type_hierarchy(type))
            if (ancestor.get_bases().empty())
            {
                urd::append(parameter_data->types, this->self().copy(ancestor));
                break;
            }
    auto variable_data = formalism::checkout<formalism::Variable>(this->copy_context().builder);
    variable_data->name = urd::flat_string("?arg");
    parameter_data->variable = this->copy_context().storage->repository.insert(*variable_data, this->copy_context().storage->builder).first.index();
    auto predicate_data = formalism::checkout<formalism::Predicate>(this->copy_context().builder);
    // Named after the type: a predicate of that name is the type itself (AstBuilder::parse_predicates).
    predicate_data->name = type.get_name();
    predicate_data->parameters.push_back(this->copy_context().storage->repository.insert(*parameter_data, this->copy_context().storage->builder).first.index());
    auto predicate = this->copy_context().storage->repository.insert(*predicate_data, this->copy_context().storage->builder).first;
    this->copy_context().type_predicates.emplace(type, predicate);
    this->copy_context().used_predicate_names.insert(std::string(type.get_name()));
    return predicate;
}

template<typename Derived>
formalism::LiteralView CompileTypingTranslator<Derived>::type_literal(formalism::TypeView type, formalism::TermView term)
{
    const auto predicate = this->self().type_predicate(type);
    auto atom_data = formalism::checkout<formalism::Atom>(this->copy_context().builder);
    urd::store(atom_data->predicate, predicate);
    urd::append(atom_data->terms, term);
    const auto atom = this->copy_context().storage->repository.insert(*atom_data, this->copy_context().storage->builder).first.index();
    auto literal_data = formalism::checkout<formalism::Literal>(this->copy_context().builder);
    literal_data->atom = atom;
    literal_data->polarity = true;
    return this->copy_context().storage->repository.insert(*literal_data, this->copy_context().storage->builder).first;
}

template<typename Derived>
formalism::ConditionView CompileTypingTranslator<Derived>::type_condition(formalism::TypeView type, formalism::VariableView variable)
{
    const auto literal = this->self().type_literal(type, this->self().copy(variable));
    auto condition_data = formalism::checkout<formalism::Condition<formalism::LiteralTag>>(this->copy_context().builder);
    urd::store(condition_data->literal, literal);
    return formalism::ConditionView(this->copy_context().storage->repository.insert(*condition_data, this->copy_context().storage->builder).first);
}

template<typename Derived>
void CompileTypingTranslator<Derived>::type_conditions_for_parameters(formalism::EntityListView<formalism::Parameter> parameters,
                                                                      urd::builder<formalism::Condition<formalism::AndTag>, formalism::Repository>& data)
{
    auto add_conditions = [&](auto&& self, formalism::TypeView type, formalism::VariableView variable) -> void
    {
        this->self().append_conjunct(data, this->self().type_condition(type, variable));
        for (auto base : type.get_bases())
            self(self, base, variable);
    };
    for (auto parameter : parameters)
        for (auto type : parameter.get_types())
            add_conditions(add_conditions, type, parameter.get_variable());
}

template<typename Derived>
void CompileTypingTranslator<Derived>::prepend_type_conditions(std::optional<formalism::ConditionView>& condition,
                                                               formalism::EntityListView<formalism::Parameter> parameters)
{
    auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
    this->self().type_conditions_for_parameters(parameters, *data);
    if (data->conditions.empty())
        return;
    if (condition)
        this->self().append_conjunct(*data, *condition);
    condition = this->self().make_conjunction(*data);
}

template<typename Derived>
void CompileTypingTranslator<Derived>::prepend_type_conditions(formalism::ConditionView& condition, formalism::EntityListView<formalism::Parameter> parameters)
{
    auto optional = std::optional { condition };
    this->self().prepend_type_conditions(optional, parameters);
    condition = *optional;
}

template<typename Derived>
formalism::ConditionView CompileTypingTranslator<Derived>::guard_with_types(std::optional<formalism::ConditionView> condition,
                                                                            formalism::EntityListView<formalism::Parameter> parameters)
{
    auto& context = this->copy_context();
    // The disjuncts: each type condition of the parameters (with the types' bases) negated, then the condition.
    auto disjuncts = urd::Store<urd::sequence<formalism::ConditionView>> {};
    auto literal_data = formalism::checkout<formalism::Literal>(context.builder);
    auto condition_data = formalism::checkout<formalism::Condition<formalism::LiteralTag>>(context.builder);
    const auto add_negated = [&](auto&& self, formalism::TypeView type, formalism::VariableView variable) -> void
    {
        const auto type_literal = this->self().type_literal(type, this->self().copy(variable));
        urd::clear_fields(*literal_data);
        literal_data->atom = type_literal.get_atom().index();
        literal_data->polarity = false;
        urd::clear_fields(*condition_data);
        condition_data->literal = context.storage->repository.insert(*literal_data, context.storage->builder).first.index();
        disjuncts.push_back(formalism::ConditionView(context.storage->repository.insert(*condition_data, context.storage->builder).first));
        for (auto base : type.get_bases())
            self(self, base, variable);
    };
    for (auto parameter : parameters)
        for (auto type : parameter.get_types())
            add_negated(add_negated, type, parameter.get_variable());
    if (disjuncts.empty())
        return condition.value();  // without a condition, called for typed parameters only
    if (condition)
        disjuncts.push_back(*condition);

    auto disjunction = formalism::checkout<formalism::Condition<formalism::OrTag>>(context.builder);
    for (auto disjunct : disjuncts)
        this->self().append_disjunct(*disjunction, disjunct);
    const auto free_parameters = this->self().free_parameters_in_scope(this->self().make_disjunction(*disjunction));

    // Compiling typing away, the derived predicate's parameters are untyped, as every other's after this phase.
    auto parameters_of_guard = urd::Store<urd::sequence<urd::index<formalism::Parameter>>> {};
    for (auto [parameter, variable] : free_parameters)
    {
        if (!this->self().compiles_typing_now())
        {
            parameters_of_guard.push_back(parameter.index());
            continue;
        }
        auto parameter_data = formalism::checkout<formalism::Parameter>(context.builder);
        parameter_data->variable = variable.index();
        parameters_of_guard.push_back(context.storage->repository.insert(*parameter_data, context.storage->builder).first.index());
    }
    auto predicate_data = formalism::checkout<formalism::Predicate>(context.builder);
    auto atom_data = formalism::checkout<formalism::Atom>(context.builder);
    for (auto [parameter, variable] : free_parameters)
        atom_data->terms.push_back(variable.index());
    for (const auto parameter : parameters_of_guard)
        predicate_data->parameters.push_back(parameter);
    predicate_data->name = urd::flat_string(this->self().next_generated_predicate_name("loki-typed-"));
    const auto predicate = context.storage->repository.insert(*predicate_data, context.storage->builder).first;
    atom_data->predicate = predicate.index();
    const auto atom = context.storage->repository.insert(*atom_data, context.storage->builder).first.index();
    urd::clear_fields(*literal_data);
    literal_data->atom = atom;
    literal_data->polarity = true;
    const auto head = context.storage->repository.insert(*literal_data, context.storage->builder).first.index();
    // Compiling typing away, each axiom has the root type conditions of its parameters, as a reparsed axiom over untyped
    // (object) parameters gets them: retranslating the output is a fixpoint.
    auto root_conditions = urd::Store<urd::sequence<formalism::ConditionView>> {};
    if (this->self().compiles_typing_now())
        for (auto [parameter, variable] : free_parameters)
            for (auto type : parameter.get_types())
                for (auto ancestor : this->self().collect_type_hierarchy(type))
                    if (ancestor.get_bases().empty())
                    {
                        urd::clear_fields(*condition_data);
                        condition_data->literal = this->self().type_literal(ancestor, variable).index();
                        root_conditions.push_back(
                            formalism::ConditionView(context.storage->repository.insert(*condition_data, context.storage->builder).first));
                    }
    for (auto disjunct : disjuncts)
    {
        auto axiom_data = formalism::checkout<formalism::Axiom>(context.builder);
        for (const auto parameter : parameters_of_guard)
            axiom_data->parameters.push_back(parameter);
        axiom_data->original_arity = axiom_data->parameters.size();
        axiom_data->head = head;
        auto body = disjunct;
        if (!root_conditions.empty())
        {
            auto conjunction = formalism::checkout<formalism::Condition<formalism::AndTag>>(context.builder);
            this->self().append_conjunct(*conjunction, disjunct);
            for (auto root_condition : root_conditions)
                this->self().append_conjunct(*conjunction, root_condition);
            body = this->self().make_conjunction(*conjunction);
        }
        urd::store(axiom_data->condition, body);
        urd::append(context.generated_axioms, context.storage->repository.insert(*axiom_data, context.storage->builder).first);
    }
    urd::append(context.generated_predicates, predicate);
    urd::clear_fields(*condition_data);
    condition_data->literal = head;
    return formalism::ConditionView(context.storage->repository.insert(*condition_data, context.storage->builder).first);
}

template<typename Derived>
std::optional<formalism::ConditionView>
CompileTypingTranslator<Derived>::prepend_start_type_conditions(std::optional<formalism::ConditionView> condition,
                                                                formalism::EntityListView<formalism::Parameter> parameters)
{
    using TimedView = formalism::EntityView<formalism::Condition<formalism::TimedTag>>;
    auto types = std::optional<formalism::ConditionView> {};
    this->self().prepend_type_conditions(types, parameters);
    if (!types)
        return condition;

    // The condition is a timed condition or a conjunction of them (AstBuilder::group_timed_condition).
    auto parts = urd::Store<urd::sequence<formalism::ConditionView>> {};
    if (condition)
    {
        if (const auto* conjunction = std::get_if<formalism::EntityView<formalism::Condition<formalism::AndTag>>>(&*condition))
            for (auto part : conjunction->get_conditions())
                parts.push_back(part);
        else
            parts.push_back(*condition);
    }
    auto start = std::optional<formalism::ConditionView> {};
    auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
    for (auto part : parts)
    {
        const auto* timed = std::get_if<TimedView>(&part);
        if (timed && timed->get_time() == formalism::TimeSpecifier::Start)
            start = timed->get_condition();
        else
            this->self().append_conjunct(*data, part);
    }
    auto start_condition = *types;
    if (start)
    {
        auto start_data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
        this->self().append_conjunct(*start_data, *types);
        this->self().append_conjunct(*start_data, *start);
        start_condition = this->self().make_conjunction(*start_data);
    }
    auto timed_data = formalism::checkout<formalism::Condition<formalism::TimedTag>>(this->copy_context().builder);
    timed_data->time = formalism::TimeSpecifier::Start;
    urd::store(timed_data->condition, start_condition);
    this->self().append_conjunct(
        *data,
        formalism::ConditionView(this->copy_context().storage->repository.insert(*timed_data, this->copy_context().storage->builder).first));
    return this->self().make_conjunction(*data);
}

template<typename Derived>
void CompileTypingTranslator<Derived>::compile_typing_to_domain(urd::builder<formalism::Domain, formalism::Repository>& data, formalism::DomainView domain)
{
    auto existing = urd::Store<urd::set<formalism::PredicateView>> {};
    for (auto predicate : domain.get_predicates())
        existing.insert(predicate);

    auto add_type = [&](auto&& self, formalism::TypeView type) -> void
    {
        const auto predicate = this->self().type_predicate(type);
        if (existing.insert(predicate).second)
            data.predicates.push_back(predicate.index());
        for (auto base : type.get_bases())
            self(self, base);
    };

    for (auto type : domain.get_types())
        add_type(add_type, type);
    for (auto object : domain.get_constants())
        for (auto type : object.get_types())
            add_type(add_type, type);

    if (this->self().compiles_typing_now())
        data.types.clear();
}

template<typename Derived>
void CompileTypingTranslator<Derived>::add_type_literals_for_object(
    urd::built_t<urd::flat_vector<urd::index<formalism::Literal>>, formalism::Repository>& literals,
    formalism::ObjectView object)
{
    const auto copied_object = this->self().copy(object);
    auto add_literal = [&](auto&& self, formalism::TypeView type) -> void
    {
        urd::append(literals, this->self().type_literal(type, copied_object));
        for (auto base : type.get_bases())
            self(self, base);
    };
    auto has_source_types = false;
    for (auto type : object.get_types())
    {
        has_source_types = true;
        add_literal(add_literal, type);
    }
    if (!has_source_types)
    {
        if (auto it = this->copy_context().storage->object_type_views.find(copied_object); it != this->copy_context().storage->object_type_views.end())
        {
            for (auto type : it->second)
                add_literal(add_literal, type);
        }
    }
}

template<typename Derived>
void CompileTypingTranslator<Derived>::initialize_type_literals(urd::builder<formalism::Task, formalism::Repository>& data, formalism::TaskView task)
{
    for (auto object : this->copy_context().storage->translated().get_constants())
        this->self().add_type_literals_for_object(data.initial_literals, object);
    for (auto object : task.get_objects())
        this->self().add_type_literals_for_object(data.initial_literals, object);

    // The task's own types need type predicates; a reparsed translation may declare them already.
    auto existing = urd::Store<urd::set<urd::index<formalism::Predicate>>> {};
    for (const auto& predicate : data.predicates)
        existing.insert(std::get<0>(predicate));  // its predicates are interned
    for (auto type : task.get_types())
        if (const auto predicate = this->self().type_predicate(type).index(); existing.insert(predicate).second)
            data.predicates.push_back(predicate);
    if (this->self().compiles_typing_now())
        data.types.clear();
}

}  // namespace loki::semantic::detail

#endif
