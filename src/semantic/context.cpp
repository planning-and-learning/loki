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

#include "context.hpp"

#include "loki/formalism/repository.hpp"
#include "loki/semantic/translator/common.hpp"
#include "mappings.hpp"

#include <string_view>
#include <utility>

namespace loki::semantic
{

namespace
{

formalism::TypeView make_base_type(formalism::Repository& repository, std::string_view name)
{
    auto builder = formalism::Builder {};
    auto data = formalism::checkout<formalism::Type>(builder);
    data->name = cista::offset::string(name);
    return formalism::insert(repository, *data).first;
}

}

DomainContext::DomainContext(std::shared_ptr<detail::TranslationStorage> storage_) :
    storage(std::move(storage_)),
    object_type(make_base_type(storage->repository, "object")),
    number_type(make_base_type(storage->repository, "number"))
{
    types.emplace("object", object_type);
    types.emplace("number", number_type);
}

void remember_requirement(ParseContext& parse_context, formalism::RequirementKind kind)
{
    for (const auto capability : requirement_capabilities(kind))
        parse_context.active_requirements.insert(capability);
}

formalism::TypeView intern_type(DomainContext& domain_context,
                                formalism::Builder& builder,
                                formalism::Repository& repository,
                                const std::string& name,
                                const std::vector<formalism::TypeView>& bases)
{
    auto k = key(name);
    if (auto it = domain_context.types.find(k); it != domain_context.types.end() && bases.empty())
        return it->second;
    auto data = formalism::checkout<formalism::Type>(builder);
    data->name = to_cista(k);
    data->bases.reserve(bases.size());
    for (const auto base : bases)
        data->bases.push_back(base.get_index());
    auto view = formalism::insert(repository, *data).first;
    if (auto [it, inserted] = domain_context.types.emplace(k, view); !inserted)
        it->second = view;
    return view;
}

void rebuild_domain_symbols(DomainContext& domain_context, formalism::Repository& repository)
{
    domain_context.types.clear();
    domain_context.objects.clear();
    domain_context.predicates.clear();
    domain_context.functions.clear();
    domain_context.declared_types.clear();
    domain_context.declared_objects.clear();
    domain_context.declared_predicates.clear();
    domain_context.declared_functions.clear();
    domain_context.requirement_kinds.clear();
    domain_context.action_costs = false;
    domain_context.numeric_fluents = false;
    if (!domain_context.domain)
        return;

    auto builder = formalism::Builder {};

    auto remember_type = [&](auto&& self, formalism::TypeView type) -> void
    {
        if (auto [it, inserted] = domain_context.types.emplace(std::string(type.get_name()), type); !inserted)
            it->second = type;
        domain_context.declared_types.insert(std::string(type.get_name()));
        for (auto base : type.get_bases())
            self(self, base);
    };

    auto parse_context = ParseContext {};
    for (auto requirement : domain_context.domain->get_requirements())
        remember_requirement(parse_context, requirement.get_kind());
    domain_context.requirement_kinds = parse_context.active_requirements;
    for (auto type : domain_context.domain->get_types())
        remember_type(remember_type, type);
    for (auto object : domain_context.domain->get_constants())
    {
        domain_context.objects.emplace(std::string(object.get_name()), object);
        domain_context.declared_objects.insert(std::string(object.get_name()));
        for (auto type : object.get_types())
            remember_type(remember_type, type);
    }
    for (auto predicate : domain_context.domain->get_predicates())
    {
        const auto name = std::string(predicate.get_name());
        domain_context.predicates.emplace(name, predicate);
        domain_context.declared_predicates.insert(name);
    }
    for (auto function : domain_context.domain->get_functions())
    {
        domain_context.functions.emplace(std::string(function.get_name()), function);
        domain_context.declared_functions.insert(std::string(function.get_name()));
        remember_type(remember_type, function.get_type());
    }
    if (auto it = domain_context.types.find("object"); it != domain_context.types.end())
        domain_context.object_type = it->second;
    else
        domain_context.object_type = intern_type(domain_context, builder, repository, "object", {});

    if (auto it = domain_context.types.find("number"); it != domain_context.types.end())
        domain_context.number_type = it->second;
    else
        domain_context.number_type = intern_type(domain_context, builder, repository, "number", {});
}

}  // namespace loki::semantic
