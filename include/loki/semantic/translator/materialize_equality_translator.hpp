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

#ifndef LOKI_SEMANTIC_TRANSLATOR_MATERIALIZE_EQUALITY_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_MATERIALIZE_EQUALITY_TRANSLATOR_HPP_

#include "loki/semantic/errors.hpp"
#include "loki/semantic/translator/copy_translator_component.hpp"

namespace loki::semantic::detail
{

template<typename Derived>
class MaterializeEqualityTranslator : public CopyTranslatorComponent<Derived, MaterializeEqualityTranslator<Derived>>
{
public:
    explicit MaterializeEqualityTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, MaterializeEqualityTranslator<Derived>>(context) {}

    bool has_requirement(formalism::EntityListView<formalism::Requirement> requirements, formalism::RequirementKind kind) const;
    bool literal_uses_equality(formalism::LiteralView literal) const;
    bool condition_uses_equality(formalism::ConditionView condition) const;
    bool effect_uses_equality(formalism::EffectView effect) const;
    bool equality_required(formalism::TaskView task) const;
    std::optional<formalism::PredicateView> find_domain_equality_predicate() const;
    formalism::LiteralView equality_literal(formalism::PredicateView predicate, ygg::Index<formalism::Object> object);
    bool domain_uses_equality(formalism::DomainView domain) const;
    void add_equality_predicate_to_domain(ygg::Data<formalism::Domain>& data, formalism::DomainView domain);
    void materialize_equality(ygg::Data<formalism::Task>& data, formalism::TaskView task);
};

template<typename Derived>
bool MaterializeEqualityTranslator<Derived>::has_requirement(formalism::EntityListView<formalism::Requirement> requirements,
                                                             formalism::RequirementKind kind) const
{
    for (auto requirement : requirements)
        if (requirement.get_kind() == kind)
            return true;
    return false;
}

template<typename Derived>
bool MaterializeEqualityTranslator<Derived>::literal_uses_equality(formalism::LiteralView literal) const
{
    return std::string_view(literal.get_atom().get_predicate().get_name()) == "=";
}

template<typename Derived>
bool MaterializeEqualityTranslator<Derived>::condition_uses_equality(formalism::ConditionView condition) const
{
    return ygg::visit(
        [&](const auto& node) -> bool
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::ConditionLiteralView>)
                return this->self().literal_uses_equality(node.get_literal());
            else if constexpr (std::is_same_v<Node, formalism::ConditionAndView> || std::is_same_v<Node, formalism::ConditionOrView>)
            {
                for (auto child : node.get_conditions())
                    if (this->self().condition_uses_equality(child))
                        return true;
                return false;
            }
            else if constexpr (std::is_same_v<Node, formalism::ConditionNotView>)
            {
                return this->self().condition_uses_equality(node.get_condition());
            }
            else if constexpr (std::is_same_v<Node, formalism::ConditionImplyView>)
            {
                return this->self().condition_uses_equality(node.get_left()) || this->self().condition_uses_equality(node.get_right());
            }
            else if constexpr (std::is_same_v<Node, formalism::ConditionExistsView> || std::is_same_v<Node, formalism::ConditionForallView>)
            {
                return this->self().condition_uses_equality(node.get_condition());
            }
            else
            {
                return false;
            }
        },
        condition.get_variant());
}

template<typename Derived>
bool MaterializeEqualityTranslator<Derived>::effect_uses_equality(formalism::EffectView effect) const
{
    return ygg::visit(
        [&](const auto& node) -> bool
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::EffectLiteralView>)
                return this->self().literal_uses_equality(node.get_literal());
            else if constexpr (std::is_same_v<Node, formalism::EffectAndView> || std::is_same_v<Node, formalism::EffectOneOfView>)
            {
                for (auto child : node.get_effects())
                    if (this->self().effect_uses_equality(child))
                        return true;
                return false;
            }
            else if constexpr (std::is_same_v<Node, formalism::EffectForallView>)
            {
                return this->self().effect_uses_equality(node.get_effect());
            }
            else if constexpr (std::is_same_v<Node, formalism::EffectWhenView>)
            {
                return this->self().condition_uses_equality(node.get_condition()) || this->self().effect_uses_equality(node.get_effect());
            }
            else if constexpr (std::is_same_v<Node, formalism::EffectProbabilisticView>)
            {
                for (auto alternative : node.get_alternatives())
                    if (this->self().effect_uses_equality(alternative.get_effect()))
                        return true;
                return false;
            }
            else if constexpr (std::is_same_v<Node, formalism::EffectProbabilisticAlternativeView>)
            {
                return this->self().effect_uses_equality(node.get_effect());
            }
            else
            {
                return false;
            }
        },
        effect.get_variant());
}

template<typename Derived>
bool MaterializeEqualityTranslator<Derived>::equality_required(formalism::TaskView task) const
{
    if (this->self().has_requirement(task.get_requirements(), formalism::RequirementKind::Equality))
        return true;
    return this->self().has_requirement(this->m_context.storage->translated_domain->get_requirements(), formalism::RequirementKind::Equality)
           || this->self().domain_uses_equality(*this->m_context.storage->translated_domain);
}

template<typename Derived>
std::optional<formalism::PredicateView> MaterializeEqualityTranslator<Derived>::find_domain_equality_predicate() const
{
    for (auto predicate : this->m_context.storage->translated_domain->get_predicates())
    {
        if (std::string_view(predicate.get_name()) == "=")
            return predicate;
    }
    return {};
}

template<typename Derived>
formalism::LiteralView MaterializeEqualityTranslator<Derived>::equality_literal(formalism::PredicateView predicate, ygg::Index<formalism::Object> object)
{
    auto term_data = formalism::checkout<formalism::Term>(this->m_context.builder);
    term_data->variant = ygg::Data<formalism::Term>::Variant(object);
    const auto term = formalism::insert(this->m_context.storage->repository, *term_data).first.get_index();
    auto atom_data = formalism::checkout<formalism::Atom>(this->m_context.builder);
    atom_data->predicate = predicate.get_index();
    atom_data->terms.push_back(term);
    atom_data->terms.push_back(term);
    const auto atom = formalism::insert(this->m_context.storage->repository, *atom_data).first.get_index();
    auto literal_data = formalism::checkout<formalism::Literal>(this->m_context.builder);
    literal_data->atom = atom;
    literal_data->m_polarity = true;
    return formalism::insert(this->m_context.storage->repository, *literal_data).first;
}

template<typename Derived>
bool MaterializeEqualityTranslator<Derived>::domain_uses_equality(formalism::DomainView domain) const
{
    for (auto predicate : domain.get_predicates())
    {
        if (std::string_view(predicate.get_name()) == "=")
            return true;
    }
    for (auto action : domain.get_actions())
    {
        if (const auto precondition = action.get_precondition())
            if (this->self().condition_uses_equality(precondition.value()))
                return true;
        if (const auto effect = action.get_effect())
            if (this->self().effect_uses_equality(effect.value()))
                return true;
    }
    for (auto axiom : domain.get_axioms())
    {
        if (this->self().literal_uses_equality(axiom.get_head()) || this->self().condition_uses_equality(axiom.get_condition()))
            return true;
    }
    return false;
}

template<typename Derived>
void MaterializeEqualityTranslator<Derived>::add_equality_predicate_to_domain(ygg::Data<formalism::Domain>& data, formalism::DomainView domain)
{
    if (!this->self().has_requirement(domain.get_requirements(), formalism::RequirementKind::Equality) && !this->self().domain_uses_equality(domain))
        return;

    for (auto predicate : domain.get_predicates())
    {
        if (std::string_view(predicate.get_name()) == "=")
        {
            data.predicates.push_back(predicate.get_index());
            return;
        }
    }

    auto object_type = std::optional<ygg::Index<formalism::Type>> {};
    for (auto type : domain.get_types())
    {
        if (std::string_view(type.get_name()) == "object")
        {
            object_type = type.get_index();
            break;
        }
    }

    auto variable_data = formalism::checkout<formalism::Variable>(this->m_context.builder);
    variable_data->name = cista::offset::string("?lhs");
    const auto left = formalism::insert(this->m_context.storage->repository, *variable_data).first.get_index();
    variable_data->clear();
    variable_data->name = cista::offset::string("?rhs");
    const auto right = formalism::insert(this->m_context.storage->repository, *variable_data).first.get_index();
    auto parameter_data = formalism::checkout<formalism::Parameter>(this->m_context.builder);
    parameter_data->variable = left;
    if (object_type)
        parameter_data->types.push_back(*object_type);
    auto predicate_data = formalism::checkout<formalism::Predicate>(this->m_context.builder);
    predicate_data->name = cista::offset::string("=");
    predicate_data->parameters.push_back(formalism::insert(this->m_context.storage->repository, *parameter_data).first.get_index());
    parameter_data->clear();
    parameter_data->variable = right;
    if (object_type)
        parameter_data->types.push_back(*object_type);
    predicate_data->parameters.push_back(formalism::insert(this->m_context.storage->repository, *parameter_data).first.get_index());
    const auto predicate = formalism::insert(this->m_context.storage->repository, *predicate_data).first.get_index();
    data.predicates.push_back(predicate);
}

template<typename Derived>
void MaterializeEqualityTranslator<Derived>::materialize_equality(ygg::Data<formalism::Task>& data, formalism::TaskView task)
{
    if (!this->self().equality_required(task))
        return;
    const auto predicate = this->self().find_domain_equality_predicate();
    if (!predicate)
        throw InvalidEqualityError("expected equality predicate to be declared in the translated domain");
    for (auto object : this->m_context.storage->translated_domain->get_constants())
        data.initial_literals.push_back(as_index(this->self().equality_literal(*predicate, object.get_index())));
    for (auto object : data.objects)
        data.initial_literals.push_back(as_index(this->self().equality_literal(*predicate, object)));
    data.domain = this->m_context.storage->translated_domain->get_index();
}

}  // namespace loki::semantic::detail

#endif
