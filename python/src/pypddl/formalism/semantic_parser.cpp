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

#include "module.hpp"

#include <filesystem>
#include <loki/formalism/domain_view.hpp>
#include <loki/formalism/repository.hpp>
#include <loki/formalism/task_view.hpp>
#include <loki/semantic/parser.hpp>
#include <nanobind/stl/filesystem.h>
#include <nanobind/stl/string.h>
#include <string>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_semantic_parser(nb::module_& m)
{
    using namespace nb::literals;
    nb::class_<semantic::ParserOptions>(m, "ParserOptions", "Options controlling semantic parser validation.")
        .def(nb::init<>())
        .def_rw("strict", &semantic::ParserOptions::strict, "Enable stricter semantic validation for requirements, arity, and type compatibility.")
        .def_rw("add_action_costs", &semantic::ParserOptions::add_action_costs, "Complete missing :action-costs artifacts while parsing.");

    nb::class_<semantic::Parser>(m, "Parser", "Parse a PDDL domain once and parse matching tasks against it.")
        .def(nb::init<const std::string&, semantic::ParserOptions>(), "domain_source"_a, "options"_a, "Parse a PDDL domain from a source string.")
        .def(nb::init<const std::filesystem::path&, semantic::ParserOptions>(), "domain_path"_a, "options"_a, "Parse a PDDL domain from a filesystem path.")
        .def("domain", &semantic::Parser::get_domain, nb::keep_alive<0, 1>(), "Return the parsed domain view.")
        .def("repository",
             nb::overload_cast<>(&semantic::Parser::repository),
             nb::rv_policy::reference_internal,
             "Return the repository that owns parsed domain and task objects.")
        .def("parse_task",
             nb::overload_cast<const std::string&>(&semantic::Parser::parse_task),
             "source"_a,
             nb::keep_alive<0, 1>(),
             "Parse a PDDL problem from a source string using this parser domain.")
        .def("parse_task",
             nb::overload_cast<const std::filesystem::path&>(&semantic::Parser::parse_task),
             "path"_a,
             nb::keep_alive<0, 1>(),
             "Parse a PDDL problem from a filesystem path using this parser domain.");
}

}  // namespace loki::formalism
