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

#include "loki/semantic/translator/copy_translator_component.hpp"

namespace loki::semantic::detail
{

template<typename Derived>
class CompileTypingTranslator : public CopyTranslatorComponent<Derived, CompileTypingTranslator<Derived>>
{
public:
    explicit CompileTypingTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, CompileTypingTranslator<Derived>>(context) {}

    bool compiles_typing_now() const noexcept;
    std::vector<formalism::TypeView> collect_type_hierarchy(formalism::TypeView type);
    void copy_type_hierarchy(formalism::EntityListView<formalism::Type> types, ygg::IndexList<formalism::Type>& result);
    void copy_parameters_without_types(formalism::EntityListView<formalism::Parameter> parameters, ygg::IndexList<formalism::Parameter>& result);
    formalism::PredicateView type_predicate(formalism::TypeView type);
    formalism::LiteralView type_literal(formalism::TypeView type, ygg::Index<formalism::Term> term);
    formalism::ConditionView type_condition(formalism::TypeView type, formalism::VariableView variable);
    void type_conditions_for_parameters(formalism::EntityListView<formalism::Parameter> parameters, ygg::Data<formalism::ConditionAnd>& data);
    void prepend_type_conditions(std::optional<formalism::ConditionView>& condition, formalism::EntityListView<formalism::Parameter> parameters);
    void prepend_type_conditions(formalism::ConditionView& condition, formalism::EntityListView<formalism::Parameter> parameters);
    void compile_typing_to_domain(ygg::Data<formalism::Domain>& data, formalism::DomainView domain);
    void add_type_literals_for_object(ygg::IndexList<formalism::Literal>& literals, formalism::ObjectView object);
    void initialize_type_literals(ygg::Data<formalism::Task>& data, formalism::TaskView task);
};

template<typename Derived>
bool CompileTypingTranslator<Derived>::compiles_typing_now() const noexcept
{
    return this->m_compile_typing && this->m_phase == TranslationPhase::CompileTyping;
}

template<typename Derived>
std::vector<formalism::TypeView> CompileTypingTranslator<Derived>::collect_type_hierarchy(formalism::TypeView type)
{
    auto result = std::vector<formalism::TypeView> {};
    auto seen = ygg::UnorderedSet<formalism::TypeView> {};
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
void CompileTypingTranslator<Derived>::copy_type_hierarchy(formalism::EntityListView<formalism::Type> types, ygg::IndexList<formalism::Type>& result)
{
    auto seen = ygg::UnorderedSet<formalism::TypeView> {};
    for (auto type : types)
    {
        for (auto collected : this->self().collect_type_hierarchy(type))
        {
            if (seen.insert(collected).second)
                result.push_back(as_index(this->self().copy(collected)));
        }
    }
}

template<typename Derived>
void CompileTypingTranslator<Derived>::copy_parameters_without_types(formalism::EntityListView<formalism::Parameter> parameters,
                                                                     ygg::IndexList<formalism::Parameter>& result)
{
    for (auto parameter : parameters)
    {
        const auto variable = as_index(this->self().copy(parameter.get_variable()));
        auto data = formalism::checkout<formalism::Parameter>(this->m_context.builder);
        data->variable = variable;
        result.push_back(formalism::insert(this->m_storage->repository, *data).first.get_index());
    }
}

template<typename Derived>
formalism::PredicateView CompileTypingTranslator<Derived>::type_predicate(formalism::TypeView type)
{
    if (auto it = this->m_type_predicates.find(type); it != this->m_type_predicates.end())
        return it->second;

    auto parameter_data = formalism::checkout<formalism::Parameter>(this->m_context.builder);
    if (!this->self().compiles_typing_now())
        parameter_data->types.push_back(as_index(this->self().copy(type)));
    auto variable_data = formalism::checkout<formalism::Variable>(this->m_context.builder);
    variable_data->name = cista::offset::string("?arg");
    parameter_data->variable = formalism::insert(this->m_storage->repository, *variable_data).first.get_index();
    auto predicate_data = formalism::checkout<formalism::Predicate>(this->m_context.builder);
    predicate_data->name = type.get_name();
    predicate_data->parameters.push_back(formalism::insert(this->m_storage->repository, *parameter_data).first.get_index());
    auto predicate = formalism::insert(this->m_storage->repository, *predicate_data).first;
    this->m_type_predicates.emplace(type, predicate);
    this->m_used_predicate_names.insert(std::string(type.get_name()));
    return predicate;
}

template<typename Derived>
formalism::LiteralView CompileTypingTranslator<Derived>::type_literal(formalism::TypeView type, ygg::Index<formalism::Term> term)
{
    const auto predicate = as_index(this->self().type_predicate(type));
    auto atom_data = formalism::checkout<formalism::Atom>(this->m_context.builder);
    atom_data->predicate = predicate;
    atom_data->terms.push_back(term);
    const auto atom = formalism::insert(this->m_storage->repository, *atom_data).first.get_index();
    auto literal_data = formalism::checkout<formalism::Literal>(this->m_context.builder);
    literal_data->atom = atom;
    literal_data->m_polarity = true;
    return formalism::insert(this->m_storage->repository, *literal_data).first;
}

template<typename Derived>
formalism::ConditionView CompileTypingTranslator<Derived>::type_condition(formalism::TypeView type, formalism::VariableView variable)
{
    const auto copied_variable = as_index(this->self().copy(variable));
    auto term_data = formalism::checkout<formalism::Term>(this->m_context.builder);
    term_data->variant = ygg::Data<formalism::Term>::Variant(copied_variable);
    const auto term = formalism::insert(this->m_storage->repository, *term_data).first.get_index();
    const auto literal = as_index(this->self().type_literal(type, term));
    auto condition_data = formalism::checkout<formalism::ConditionLiteral>(this->m_context.builder);
    condition_data->literal = literal;
    return this->self().wrap_condition(formalism::insert(this->m_storage->repository, *condition_data).first);
}

template<typename Derived>
void CompileTypingTranslator<Derived>::type_conditions_for_parameters(formalism::EntityListView<formalism::Parameter> parameters,
                                                                      ygg::Data<formalism::ConditionAnd>& data)
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
    auto data = formalism::checkout<formalism::ConditionAnd>(this->m_context.builder);
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
void CompileTypingTranslator<Derived>::compile_typing_to_domain(ygg::Data<formalism::Domain>& data, formalism::DomainView domain)
{
    auto existing = ygg::UnorderedSet<formalism::PredicateView> {};
    for (auto predicate : domain.get_predicates())
        existing.insert(predicate);

    auto add_type = [&](auto&& self, formalism::TypeView type) -> void
    {
        const auto predicate = this->self().type_predicate(type);
        if (existing.insert(predicate).second)
            data.predicates.push_back(predicate.get_index());
        for (auto base : type.get_bases())
            self(self, base);
    };

    for (auto type : domain.get_types())
        add_type(add_type, type);
    for (auto object : domain.get_constants())
        for (auto type : object.get_types())
            add_type(add_type, type);

    this->self().strip_typing_requirement(domain.get_requirements(), data.requirements);
    if (this->self().compiles_typing_now())
        data.types.clear();
}

template<typename Derived>
void CompileTypingTranslator<Derived>::add_type_literals_for_object(ygg::IndexList<formalism::Literal>& literals, formalism::ObjectView object)
{
    const auto copied_object = this->self().copy(object);
    auto term_data = formalism::checkout<formalism::Term>(this->m_context.builder);
    term_data->variant = ygg::Data<formalism::Term>::Variant(copied_object.get_index());
    const auto term = formalism::insert(this->m_storage->repository, *term_data).first.get_index();
    auto add_literal = [&](auto&& self, formalism::TypeView type) -> void
    {
        literals.push_back(as_index(this->self().type_literal(type, term)));
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
        if (auto it = this->m_storage->object_type_views.find(copied_object); it != this->m_storage->object_type_views.end())
        {
            for (auto type : it->second)
                add_literal(add_literal, type);
        }
    }
}

template<typename Derived>
void CompileTypingTranslator<Derived>::initialize_type_literals(ygg::Data<formalism::Task>& data, formalism::TaskView task)
{
    for (auto object : this->m_storage->translated_domain->get_constants())
        this->self().add_type_literals_for_object(data.initial_literals, object);
    for (auto object : task.get_objects())
        this->self().add_type_literals_for_object(data.initial_literals, object);
    this->self().strip_typing_requirement(task.get_requirements(), data.requirements);
}

}  // namespace loki::semantic::detail

#endif
