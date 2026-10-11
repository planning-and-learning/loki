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

#include <loki/formalism/formatter.hpp>
#include <loki/formalism/repository.hpp>
#include <loki/semantic/translator.hpp>
#include <nanobind/stl/string.h>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_semantic_translator(nb::module_& m)
{
    using namespace nb::literals;
    nb::class_<semantic::TranslatorOptions>(m, "TranslatorOptions", "Options controlling PDDL normalization and translation.")
        .def(nb::init<>())
        .def_rw("compile_typing", &semantic::TranslatorOptions::compile_typing, "Compile typing away into type predicates and remove type annotations.")
        .def_rw("compile_conditional_effects",
                &semantic::TranslatorOptions::compile_conditional_effects,
                "Split actions to eliminate top-level conditional effects after effect normalization.")
        .def_rw("materialize_equality",
                &semantic::TranslatorOptions::materialize_equality,
                "Add equality predicate and equality initial literals during translation.")
        .def_rw("normalize_arithmetic_expressions",
                &semantic::TranslatorOptions::normalize_arithmetic_expressions,
                "Normalize addition and multiplication modulo associativity, commutativity, and identity elements.");

    nb::class_<semantic::DomainTranslationResult>(m, "DomainTranslationResult", "Owns the original and translated domain views produced by translate_domain.")
        .def_prop_ro("original_domain", &semantic::DomainTranslationResult::get_original_domain, nb::keep_alive<0, 1>())
        .def_prop_ro("translated_domain", &semantic::DomainTranslationResult::get_translated_domain, nb::keep_alive<0, 1>())
        .def("repository", nb::overload_cast<>(&semantic::DomainTranslationResult::get_repository), nb::rv_policy::reference_internal);

    nb::class_<semantic::ProblemTranslationResult>(m, "ProblemTranslationResult", "Owns the original and translated task views produced by translate_task.")
        .def_prop_ro("original_task", &semantic::ProblemTranslationResult::get_original_task, nb::keep_alive<0, 1>())
        .def_prop_ro("translated_task", &semantic::ProblemTranslationResult::get_translated_task, nb::keep_alive<0, 1>())
        .def("repository", nb::overload_cast<>(&semantic::ProblemTranslationResult::get_repository), nb::rv_policy::reference_internal);

    m.def(
        "translate_domain",
        [](formalism::DomainView domain, const semantic::TranslatorOptions& options) { return semantic::translate(domain, options); },
        "domain"_a,
        "options"_a,
        nb::keep_alive<0, 1>(),
        "Translate and normalize a parsed domain.");
    m.def(
        "translate_task",
        [](formalism::TaskView task, const semantic::DomainTranslationResult& domain_translation, const semantic::TranslatorOptions& options)
        { return semantic::translate(task, domain_translation, options); },
        "task"_a,
        "domain_translation"_a,
        "options"_a,
        nb::keep_alive<0, 1>(),
        nb::keep_alive<0, 2>(),
        "Translate and normalize a parsed task with a matching domain translation.");
    m.def(
        "format_domain",
        [](formalism::DomainView domain) { return formalism::format::to_string(domain); },
        "domain"_a,
        "Format a domain view as reparseable PDDL text.");
    m.def("format_task", [](formalism::TaskView task) { return formalism::format::to_string(task); }, "task"_a, "Format a task view as reparseable PDDL text.");
}

}  // namespace loki::formalism
