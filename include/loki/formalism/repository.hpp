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

#ifndef LOKI_FORMALISM_REPOSITORY_HPP_
#define LOKI_FORMALISM_REPOSITORY_HPP_

#include "loki/formalism/canonicalization.hpp"
#include "loki/formalism/datas.hpp"
#include "loki/formalism/declarations.hpp"

#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <yggdrasil/formalism/relation_repository.hpp>
#include <yggdrasil/formalism/repository.hpp>
#include <yggdrasil/formalism/repository_factory.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace loki::formalism
{

using Builder = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, SymbolRepositoryTypes>;

using ygg::formalism::checkout;
using ygg::formalism::insert;

template<typename T>
void prepare_for_insert(Repository& repository, urd::data<T>& data)
{ canonicalize(repository, data); }

/// The stored form of a function expression, condition or effect read through the repository.
inline FunctionExpressionVariant as_index(const FunctionExpressionView& view) { return urd::to_stored(view); }
inline ConditionVariant as_index(const ConditionView& view) { return urd::to_stored(view); }
inline EffectVariant as_index(const EffectView& view) { return urd::to_stored(view); }

}

extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Requirement>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Type>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Object>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Variable>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Parameter>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Predicate>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::FunctionSkeleton>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Atom>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Literal>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::FunctionExpression<loki::formalism::NumberTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::FunctionTerm>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::FunctionExpression<loki::formalism::UnaryTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::FunctionExpression<loki::formalism::BinaryTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::FunctionExpression<loki::formalism::MultiTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::LiteralTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::AndTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::OrTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::NotTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::ImplyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::ExistsTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::ForallTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Condition<loki::formalism::NumericConstraintTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::LiteralTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::AndTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::NumericTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::ForallTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::WhenTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::OneOfTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::EffectProbabilisticAlternative>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Effect<loki::formalism::ProbabilisticTag>>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Action>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Axiom>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Metric>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::InitialFunctionValue>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Domain>;
extern template class ygg::formalism::BasicSymbolRepository<loki::formalism::Task>;

#endif
