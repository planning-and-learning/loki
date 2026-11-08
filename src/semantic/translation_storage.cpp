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

#include "loki/formalism/repository.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "translator/canonical_copy_translator.hpp"
#include "translator/common.hpp"

#include <boost/mp11/algorithm.hpp>
#include <type_traits>
#include <utility>

namespace loki::semantic::detail
{

/// Each map of `out` becomes the composition of source_to_middle's and middle_to_target's map of its kind.
static void compose_maps(ViewMaps& out, const ViewMaps& source_to_middle, const ViewMaps& middle_to_target)
{
    out.for_each(
        [&](auto& map)
        {
            using Map = std::remove_cvref_t<decltype(map)>;
            using T = typename Map::key_type::value_type;
            const auto& second = middle_to_target.template get<T>();
            auto composed = Map();
            for (const auto& [source, middle] : source_to_middle.template get<T>())
            {
                if (auto it = second.find(middle); it != second.end())
                    composed.emplace(source, it->second);
            }
            // Either input may alias out; replace the map only after reading both.
            map = std::move(composed);
        });
}

static formalism::TypeView copy_type_view_for_metadata(TranslationStorage& target, formalism::TypeView source)
{
    return memoized(target.maps,
                    source,
                    [&] { return rebuild(target.repository, source, [&](formalism::TypeView base) { return copy_type_view_for_metadata(target, base); }); });
}

static void remap_object_type_metadata(TranslationStorage& target,
                                       const TranslationStorage& source,
                                       const ViewMap<formalism::Object>& object_map,
                                       const ViewMap<formalism::Type>& type_map)
{
    target.object_type_views.clear();
    for (const auto& [source_object, type_views] : source.object_type_views)
    {
        auto object_it = object_map.find(source_object);
        if (object_it == object_map.end())
            continue;

        auto remapped = urd::Store<urd::sequence<formalism::TypeView>> {};
        for (auto type : type_views)
        {
            if (auto type_it = type_map.find(type); type_it != type_map.end())
                remapped.push_back(type_it->second);
            else
                remapped.push_back(copy_type_view_for_metadata(target, type));
        }
        target.object_type_views.emplace(object_it->second, std::move(remapped));
    }
}

std::shared_ptr<TranslationStorage> canonicalize_domain_storage(formalism::DomainView original_domain, const std::shared_ptr<TranslationStorage>& middle)
{
    auto canonical = std::make_shared<TranslationStorage>();
    const auto middle_domain = middle->maps.get<formalism::Domain>().at(original_domain);
    auto context = CanonicalCopyTranslator(canonical);
    const auto canonical_domain = copy(middle_domain, context).first;

    const auto middle_objects = canonical->maps.get<formalism::Object>();
    const auto middle_types = canonical->maps.get<formalism::Type>();
    remap_object_type_metadata(*canonical, *middle, middle_objects, middle_types);

    canonical->translated_domain = canonical_domain;
    compose_maps(canonical->maps, middle->maps, canonical->maps);
    // The composed domain map is replaced: it maps the original domain alone.
    auto& domains = canonical->maps.get<formalism::Domain>();
    domains.clear();
    domains.emplace(original_domain, canonical_domain);
    return canonical;
}

void compose_storage_maps_from_previous(TranslationStorage& target, const TranslationStorage& previous)
{
    const auto precompose_objects = target.maps.get<formalism::Object>();
    const auto precompose_types = target.maps.get<formalism::Type>();
    remap_object_type_metadata(target, previous, precompose_objects, precompose_types);

    compose_maps(target.maps, previous.maps, target.maps);
}

/// The kinds of entities a problem's translation takes over from its domain's, besides the domain.
using DomainKinds = boost::mp11::mp_list<formalism::Requirement, formalism::Type, formalism::Object, formalism::Predicate, formalism::FunctionSkeleton>;

/// Calls `function` with the problem's and the domain's map of each of the DomainKinds.
template<typename Function>
static void for_each_domain_kind(ViewMaps& problem, const ViewMaps& domain, Function function)
{
    boost::mp11::mp_for_each<boost::mp11::mp_transform<boost::mp11::mp_identity, DomainKinds>>(
        [&](auto kind)
        {
            using T = typename decltype(kind)::type;
            function(problem.template get<T>(), domain.template get<T>());
        });
}

void inherit_domain_mappings(TranslationStorage& problem, const TranslationStorage& domain)
{
    problem.maps.get<formalism::Domain>() = domain.maps.get<formalism::Domain>();
    for_each_domain_kind(problem.maps, domain.maps, [](auto& target, const auto& source) { target = source; });
    problem.object_type_views = domain.object_type_views;
    problem.translated_domain = domain.translated_domain;
}

void inherit_domain_identity_mappings(TranslationStorage& problem, const TranslationStorage& domain)
{
    remember(problem.maps, domain.translated(), domain.translated());
    for_each_domain_kind(problem.maps,
                         domain.maps,
                         [](auto& target, const auto& source)
                         {
                             for (const auto& [_, view] : source)
                                 target.emplace(view, view);
                         });
    problem.object_type_views = domain.object_type_views;
    problem.translated_domain = domain.translated_domain;
}

std::shared_ptr<TranslationStorage> canonicalize_problem_storage(formalism::TaskView middle_task,
                                                                 const std::shared_ptr<TranslationStorage>& middle,
                                                                 const std::shared_ptr<const TranslationStorage>& domain)
{
    auto canonical = std::make_shared<TranslationStorage>(domain);
    inherit_domain_identity_mappings(*canonical, *domain);
    if (middle->translated().index() == domain->translated().index())
        remember(canonical->maps, middle->translated(), domain->translated());
    auto context = CanonicalCopyTranslator(canonical);
    copy(middle_task, context);
    compose_storage_maps_from_previous(*canonical, *middle);
    return canonical;
}

}  // namespace loki::semantic::detail
