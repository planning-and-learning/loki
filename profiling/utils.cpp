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

#include "utils.hpp"

#include <string>
#include <utility>
#include <vector>

namespace loki::benchmarks
{
namespace
{
template<typename T, typename Initialize>
formalism::EntityView<T> intern(formalism::Repository& repository, formalism::Builder& builder, Initialize&& initialize)
{
    auto data = formalism::checkout<T>(builder);
    std::forward<Initialize>(initialize)(*data);
    return formalism::insert(repository, *data).first;
}
}  // namespace

AtomList create_atoms(size_t num_objects, size_t num_predicates, formalism::Repository& repository, formalism::Builder& builder)
{
    auto objects = std::vector<formalism::ObjectView> {};
    objects.reserve(num_objects);
    for (size_t i = 1; i <= num_objects; ++i)
        objects.push_back(intern<formalism::Object>(repository, builder, [&](auto& data) { data.name = "object_" + std::to_string(i); }));

    const auto parameter = [&](const char* name)
    {
        const auto variable = intern<formalism::Variable>(repository, builder, [&](auto& data) { data.name = name; });
        return intern<formalism::Parameter>(repository, builder, [&](auto& data) { data.variable = variable.get_index(); }).get_index();
    };
    const auto parameters = ygg::IndexList<formalism::Parameter> { parameter("variable_left"), parameter("variable_right") };

    auto predicates = std::vector<formalism::PredicateView> {};
    predicates.reserve(num_predicates);
    for (size_t i = 1; i <= num_predicates; ++i)
        predicates.push_back(intern<formalism::Predicate>(repository,
                                                          builder,
                                                          [&](auto& data)
                                                          {
                                                              data.name = "predicate_" + std::to_string(i);
                                                              data.parameters = parameters;
                                                          }));

    const auto term = [&](formalism::ObjectView object)
    {
        return intern<formalism::Term>(repository, builder, [&](auto& data) { data.variant = ygg::Data<formalism::Term>::Variant(object.get_index()); })
            .get_index();
    };
    auto atoms = AtomList {};
    atoms.reserve(num_objects * num_objects * num_predicates);
    for (const auto& predicate : predicates)
        for (const auto& object_left : objects)
            for (const auto& object_right : objects)
                atoms.push_back(intern<formalism::Atom>(repository,
                                                        builder,
                                                        [&](auto& data)
                                                        {
                                                            data.predicate = predicate.get_index();
                                                            data.terms = ygg::IndexList<formalism::Term> { term(object_left), term(object_right) };
                                                        }));
    return atoms;
}

}
