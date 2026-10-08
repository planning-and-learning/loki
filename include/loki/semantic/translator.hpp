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

#ifndef LOKI_SEMANTIC_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_HPP_

#include "loki/formalism/domain_view.hpp"
#include "loki/formalism/repository.hpp"
#include "loki/formalism/task_view.hpp"
#include "loki/semantic/options.hpp"

#include <memory>

namespace loki::semantic
{

namespace detail
{
struct TranslationStorage;
}

class ProblemTranslationResult;

class DomainTranslationResult
{
public:
    DomainTranslationResult(formalism::DomainView original_domain_, std::shared_ptr<detail::TranslationStorage> storage_);

    formalism::DomainView get_original_domain() const noexcept;
    formalism::DomainView get_translated_domain() const noexcept;
    const formalism::Repository& get_repository() const noexcept;
    formalism::Repository& get_repository() noexcept;

private:
    friend class ProblemTranslationResult;
    friend ProblemTranslationResult translate(formalism::TaskView task, const DomainTranslationResult& result, const TranslatorOptions& options);

    formalism::DomainView m_original_domain;
    std::shared_ptr<detail::TranslationStorage> m_storage;
};

class ProblemTranslationResult
{
public:
    ProblemTranslationResult(formalism::TaskView original_task_, std::shared_ptr<detail::TranslationStorage> storage_, formalism::TaskView translated_task_);

    formalism::TaskView get_original_task() const noexcept;
    formalism::TaskView get_translated_task() const noexcept;
    const formalism::Repository& get_repository() const noexcept;
    formalism::Repository& get_repository() noexcept;

private:
    formalism::TaskView m_original_task;
    std::shared_ptr<detail::TranslationStorage> m_storage;
    formalism::TaskView m_translated_task;
};

DomainTranslationResult translate(formalism::DomainView domain, const TranslatorOptions& options = {});
ProblemTranslationResult translate(formalism::TaskView task, const DomainTranslationResult& result, const TranslatorOptions& options = {});

}  // namespace loki::semantic

#endif
