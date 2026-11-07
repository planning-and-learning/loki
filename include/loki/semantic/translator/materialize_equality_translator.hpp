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

    [[nodiscard]] bool has_requirement(formalism::EntityListView<formalism::Requirement> requirements, formalism::RequirementKind kind) const;
    /// Whether the predicate "=" occurs in node: it or one of its descendants.
    template<typename T>
    [[nodiscard]] bool uses_equality(formalism::EntityView<T> node) const;
    [[nodiscard]] bool uses_equality(formalism::PredicateView predicate) const { return std::string_view(predicate.get_name()) == "="; }
    [[nodiscard]] bool equality_required(formalism::TaskView task) const;
    [[nodiscard]] std::optional<formalism::PredicateView> find_domain_equality_predicate() const;
    formalism::LiteralView equality_literal(formalism::PredicateView predicate, urd::index<formalism::Object> object);
    void add_equality_predicate_to_domain(urd::data<formalism::Domain>& data, formalism::DomainView domain);
    void materialize_equality(urd::data<formalism::Task>& data, formalism::TaskView task);
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
template<typename T>
bool MaterializeEqualityTranslator<Derived>::uses_equality(formalism::EntityView<T> node) const
{
    auto found = false;
    urd::for_each_child<formalism::SymbolRepositoryTypes, T>(node.data(),
                                                             node.context(),
                                                             [&](auto child) { found = found || this->self().uses_equality(child); });
    return found;
}

template<typename Derived>
bool MaterializeEqualityTranslator<Derived>::equality_required(formalism::TaskView task) const
{
    if (this->self().has_requirement(task.get_requirements(), formalism::RequirementKind::Equality))
        return true;
    return this->self().has_requirement(this->copy_context().storage->translated().get_requirements(), formalism::RequirementKind::Equality)
           || this->self().uses_equality(this->copy_context().storage->translated());
}

template<typename Derived>
std::optional<formalism::PredicateView> MaterializeEqualityTranslator<Derived>::find_domain_equality_predicate() const
{
    for (auto predicate : this->copy_context().storage->translated().get_predicates())
    {
        if (std::string_view(predicate.get_name()) == "=")
            return predicate;
    }
    return {};
}

template<typename Derived>
formalism::LiteralView MaterializeEqualityTranslator<Derived>::equality_literal(formalism::PredicateView predicate, urd::index<formalism::Object> object)
{
    auto atom_data = formalism::checkout<formalism::Atom>(this->copy_context().builder);
    atom_data->predicate = predicate.index();
    atom_data->terms.push_back(object);
    atom_data->terms.push_back(object);
    const auto atom = formalism::insert(this->copy_context().storage->repository, *atom_data).first.index();
    auto literal_data = formalism::checkout<formalism::Literal>(this->copy_context().builder);
    literal_data->atom = atom;
    literal_data->m_polarity = true;
    return formalism::insert(this->copy_context().storage->repository, *literal_data).first;
}

template<typename Derived>
void MaterializeEqualityTranslator<Derived>::add_equality_predicate_to_domain(urd::data<formalism::Domain>& data, formalism::DomainView domain)
{
    if (!this->self().has_requirement(domain.get_requirements(), formalism::RequirementKind::Equality) && !this->self().uses_equality(domain))
        return;

    for (auto predicate : domain.get_predicates())
    {
        if (std::string_view(predicate.get_name()) == "=")
        {
            data.predicates.push_back(predicate.index());
            return;
        }
    }

    auto object_type = std::optional<urd::index<formalism::Type>> {};
    for (auto type : domain.get_types())
    {
        if (std::string_view(type.get_name()) == "object")
        {
            object_type = type.index();
            break;
        }
    }

    auto variable_data = formalism::checkout<formalism::Variable>(this->copy_context().builder);
    variable_data->name = urd::flat_string("?lhs");
    const auto left = formalism::insert(this->copy_context().storage->repository, *variable_data).first.index();
    urd::clear_members(*variable_data);
    variable_data->name = urd::flat_string("?rhs");
    const auto right = formalism::insert(this->copy_context().storage->repository, *variable_data).first.index();
    auto parameter_data = formalism::checkout<formalism::Parameter>(this->copy_context().builder);
    parameter_data->variable = left;
    if (object_type)
        parameter_data->types.push_back(*object_type);
    auto predicate_data = formalism::checkout<formalism::Predicate>(this->copy_context().builder);
    predicate_data->name = urd::flat_string("=");
    predicate_data->parameters.push_back(formalism::insert(this->copy_context().storage->repository, *parameter_data).first.index());
    urd::clear_members(*parameter_data);
    parameter_data->variable = right;
    if (object_type)
        parameter_data->types.push_back(*object_type);
    predicate_data->parameters.push_back(formalism::insert(this->copy_context().storage->repository, *parameter_data).first.index());
    const auto predicate = formalism::insert(this->copy_context().storage->repository, *predicate_data).first.index();
    data.predicates.push_back(predicate);
}

template<typename Derived>
void MaterializeEqualityTranslator<Derived>::materialize_equality(urd::data<formalism::Task>& data, formalism::TaskView task)
{
    if (!this->self().equality_required(task))
        return;
    const auto predicate = this->self().find_domain_equality_predicate();
    if (!predicate)
        throw InvalidEqualityError("expected equality predicate to be declared in the translated domain");
    for (auto object : this->copy_context().storage->translated().get_constants())
        data.initial_literals.push_back(urd::to_stored(this->self().equality_literal(*predicate, object.index())));
    for (auto object : data.objects)
        data.initial_literals.push_back(urd::to_stored(this->self().equality_literal(*predicate, object)));
    data.domain = this->copy_context().storage->translated().index();
}

}  // namespace loki::semantic::detail

#endif
