/*
 * Copyright (C) 2026 Dominik Drexler
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

#include <yggdrasil/formalism/detail/basic_symbol_repository_impl.hpp>

template class ygg::formalism::BasicSymbolRepository<loki::formalism::Requirement>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Type>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Object>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Variable>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Parameter>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Predicate>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::FunctionSkeleton>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Term>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Atom>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Literal>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::FunctionExpression<loki::formalism::NumberTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::FunctionTerm>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::FunctionExpression<loki::formalism::UnaryTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::FunctionExpression<loki::formalism::BinaryTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::FunctionExpression<loki::formalism::MultiTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::FunctionExpression<>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::LiteralTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::AndTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::OrTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::NotTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::ImplyTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::ExistsTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::ForallTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::NumericConstraintTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::LiteralTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::AndTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::NumericTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::ForallTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::WhenTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::OneOfTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::EffectProbabilisticAlternative>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::ProbabilisticTag>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<>>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Action>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Axiom>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Metric>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::InitialFunctionValue>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Domain>;
template class ygg::formalism::BasicSymbolRepository<loki::formalism::Task>;
