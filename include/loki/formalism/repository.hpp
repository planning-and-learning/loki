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
#include "loki/formalism/indices.hpp"

#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <yggdrasil/formalism/relation_repository.hpp>
#include <yggdrasil/formalism/repository.hpp>
#include <yggdrasil/formalism/repository_factory.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace loki::formalism
{

using Builder = ygg::ApplyTypeListT<ygg::formalism::BuilderStorage, SymbolRepositoryTypes>;

using ygg::formalism::checkout;
using ygg::formalism::get_or_create;

template<typename T>
void prepare_for_interning(Repository& repository, ygg::Data<T>& data)
{
    canonicalize(repository, data);
}

}

#endif
