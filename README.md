# Loki: PDDL Parsing and Translation in C++ and Python

Loki is a C++20 library with Python bindings for parsing and translation of PDDL files. Loki separates the parsing and translation of domain and problem files, allowing users to effectively work with collection of problems. 

The parser is based on the canonical parser structure proposed in the [Boost Spirit X3 library](https://beta.boost.org/doc/libs/1_82_0/libs/spirit/doc/x3/html/spirit_x3/tutorials/rexpr.html).

The translator is based on the method presented in section four of the paper [*"Concise finite-domain representations for PDDL planning tasks by Malte Helmert (AIJ 2009)"*](https://ai.dmi.unibas.ch/papers/helmert-aij2009.pdf).

## Supported PDDL Requirements

- [x] :strips
- [x] :typing
- [x] :negative-preconditions
- [x] :disjunctive-preconditions
- [x] :equality
- [x] :existential-preconditions
- [x] :universal-preconditions
- [x] :quantified-preconditions
- [x] :conditional-effects
- [x] :numeric-fluents
- [x] :adl
- [x] :derived-predicates
- [x] :action-costs
- [x] :non-deterministic (unsupported in the translator)
- [x] :probabilistic-effects (unsupported in the translator)

## Dependencies

Loki depends on a fraction of [Boost's](https://www.boost.org) header-only libraries (Fusion, Spirit x3, Container), its performance benchmarking framework depends on [GoogleBenchmark](https://github.com/google/benchmark), and its testing framework depends on [GoogleTest](https://github.com/google/googletest).

Loki consumes native dependencies from Python packages:

- `pyyggdrasil >= 0.2, < 0.3` for shared third-party native dependencies.
- `pypddl-datasets >= 0.0.9, < 0.1` for the PDDL benchmark data used by the C++ test suite and the example executables (resolved from its cache at CMake configure time).

The shared workspace layout, layered install order, and the common
build-from-source and CMake-integration patterns are documented in the
[Planning and Learning build instructions](https://github.com/planning-and-learning/.github/blob/main/profile/README.md#building-from-source);
the sections below cover `loki`/`pypddl`-specific details.

For offline/local development, install `pyyggdrasil` from the sibling source
checkout instead:

```console
cd ../yggdrasil
uv pip install --python ../loki/.venv/bin/python .
```

## Build C++

Install Loki's native dependency providers into the active Python environment,
then configure CMake with their native prefixes:

```console
python -m pip install 'pyyggdrasil>=0.2.1,<0.3' 'pypddl-datasets>=0.0.9,<0.1'

cmake -S . -B build
```

CMake discovers the installed `pyyggdrasil` automatically through
`cmake/bootstrap_pyyggdrasil.cmake` (which locates the package and adds its
native prefix to `CMAKE_PREFIX_PATH`, after which `find_package(yggdrasil)`
provides the shared helper functions) and links against the
`yggdrasil::yggdrasil` target. To point at a different prefix explicitly:

```console
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH="$(python -m pyyggdrasil --prefix)"

cmake --build build -j4
```

CMake options:

| Option | Default | Description |
| --- | --- | --- |
| `LOKI_BUILD_TESTS` | `OFF` | Build Loki tests. |
| `LOKI_BUILD_EXECUTABLES` | `OFF` | Build Loki executables. |
| `LOKI_BUILD_PROFILING` | `OFF` | Build Loki profiling targets. |
| `LOKI_BUILD_PYPDDL` | `OFF` | Build Loki for the `pypddl` Python wheel. |
| `LOKI_USE_LLD` | `ON` | Use LLVM `lld` with Clang when available. |
| `LOKI_ENABLE_LTO` | `ON` | Enable link-time optimization for Release builds. |

Single-config CMake builds default to Release. On GCC and Clang, Debug builds
use `-Og` with debug symbols, RelWithDebInfo keeps frame pointers and disables
LTO, and Release LTO uses GCC LTO or Clang ThinLTO. Editable installs and
wheels disable `LOKI_USE_LLD` and `LOKI_ENABLE_LTO` by default for build
reliability.

Run tests from a build configured with `-DLOKI_BUILD_TESTS=ON`:

```console
ctest --test-dir build --output-on-failure
```

Install Loki from a configured build directory with:

```console
cmake --install build --prefix=<path/to/installation-directory>
```

## Build Python

The Python package requires Python 3.11 or newer.

```console
python -m pip install .
```

## Python API

The Python package exposes the semantic parser, translator, and reparseable PDDL
formatter through `pypddl.formalism`:

```python
from pypddl import formalism as pddl

parser = pddl.Parser("""
(define (domain ready-domain)
  (:predicates (ready))
)
""")

translation = pddl.translate_domain(parser.domain())
domain_text = pddl.format_domain(translation.translated_domain)
reparsed = pddl.Parser(domain_text)
assert reparsed.domain().get_name() == "ready-domain"
```

With default options the parser completes `:action-costs` artifacts and the
translator compiles typing and materializes equality; see
[Parser and Translator Options](#parser-and-translator-options) to override.

Programmatic construction inserts mutable data into a repository and returns
`(view, inserted)`. Repeated insertion returns the existing view with `False`.
The returned Python view keeps its repository alive after the tuple is unpacked.

```python
repository = pddl.RepositoryFactory().create()
object_type, inserted = repository.insert(pddl.TypeData("object"))
same_type, inserted_again = repository.insert(pddl.TypeData("object"))
assert inserted and not inserted_again and object_type == same_type
```

## C++ API

The umbrella header exposes the semantic parser, translator, and reparseable
PDDL formatter through the top-level `loki` namespace:

```cpp
#include <loki/loki.hpp>

#include <string>

int main()
{
    auto parser = loki::Parser(std::string { "(define (domain ready-domain) (:predicates (ready)))" });
    const auto translation = loki::translate_domain(parser.get_domain());
    const auto domain_text = loki::format_domain(translation.get_translated_domain());
    auto reparsed = loki::Parser(domain_text);
    return reparsed.get_domain().get_name() == "ready-domain" ? 0 : 1;
}
```

For programmatic C++ construction, `loki::formalism::insert(repository, data)`
prepares the record and returns `std::pair<View, bool>`. Symbol views reuse Ygg's internal accessors and require a context whose
repository supports their specific entity type. The canonical and phase-copy contexts expose
`copy(domain_or_task, context)` with the same pair result; repeated copies reuse
the context's memoized destination values and report `false`.

## Parser and Translator Options

Both APIs accept options at construction/translation time. The library defaults
are normalization-friendly for downstream consumers; every option can be turned
off individually.

`ParserOptions` (second argument of `Parser`):

| Option | Default | Description |
| --- | --- | --- |
| `strict` | `false` | Strict semantic validation for requirements, arity, and type compatibility. Numeric-fluents violations error even in permissive mode: reads require `:fluents`/`:numeric-fluents`/`:action-costs`, and writes other than `(increase (total-cost) ...)` require `:fluents`/`:numeric-fluents`. |
| `add_action_costs` | `true` | Complete missing `:action-costs` artifacts (`total-cost` function, initial value, `minimize` metric) instead of erroring. If the domain declares neither `:action-costs` nor `:fluents`/`:numeric-fluents`, additionally injects the requirement and a unit-cost effect `(increase (total-cost) 1)` into every action that does not already write `total-cost`. Genuine numeric domains are left untouched: an absent metric means unit costs. |

The raw syntax AST remains permissive and preserves empty and unary `+`/`*`
nodes. Semantic construction always maps empty addition and multiplication to
`0` and `1`, maps unary forms to their operand, and lexicographically sorts the
immediate operands of each remaining multi-expression.

`TranslatorOptions` (second argument of `translate_domain`/`translate_task`):

| Option | Default | Description |
| --- | --- | --- |
| `compile_typing` | `true` | Compile typing away into type predicates and remove type annotations. |
| `compile_conditional_effects` | `false` | Multiply conditional effects out into unconditional actions; the normalization phases re-run afterwards so preconditions stay conjunctive (quantified `when`-conditions become derived predicates). Worst case exponential in the number of conditional effects per action. |
| `materialize_equality` | `true` | Add the `=` predicate and `(= o o)` initial literals. Turn off for consumers with native equality handling. |
| `normalize_arithmetic_expressions` | `false` | Recursively flatten addition and multiplication, remove exact identity operands, and lexicographically sort the remaining leaves while preserving duplicates. Reassociation can change IEEE-754 evaluation order and rounding. |

Pass the same `TranslatorOptions` to the domain and the task translation;
mismatched options between the two can fail (e.g. equality materialization
requires the `=` predicate added during domain translation).

```python
options = pddl.TranslatorOptions()
options.materialize_equality = False
translation = pddl.translate_domain(parser.domain(), options)
```

The `loki` executable exposes the same options as opt-in flags (all off by
default): `--strict`, `--add-action-costs`, `--compile-typing`,
`--compile-conditional-effects`, `--materialize-equality`, and
`--normalize-arithmetic-expressions`.

Migration note: semantic/formalism `MultiFunctionExpression` operands are now
named `args`; use `Data(op, args)` and `get_args()` (likewise in Python).
Arithmetic normalization folds empty and unary nodes to their identity or sole
argument, so compiled output keeps `MultiFunctionExpression` for two or more
arguments only. The native `EffectNumeric` target is now a `FunctionTerm`;
construct it with `Data(op, function_term, expression)` and access its arguments
through `get_function().get_terms()`. The native serialized layout changed, so
recreate repositories serialized by older Loki versions.

## CMake Integration

This section covers `pypddl`-specific paths and targets; the general pattern for
consuming the native prefixes from CMake is in the
[common CMake integration instructions](https://github.com/planning-and-learning/.github/blob/main/profile/README.md#cmake-integration).

The Python package `pypddl` installs Loki's native headers, shared library, and
CMake package config under `pypddl.native_prefix()`. It depends on
`pyyggdrasil>=0.2.1,<0.3` for third-party native dependencies:

```python
import pypddl
import pyyggdrasil

print(pypddl.cmake_prefix())       # prefix to put on CMAKE_PREFIX_PATH
print(pypddl.cmake_dir())          # directory containing lokiConfig.cmake
print(pyyggdrasil.cmake_prefix())
```

The same paths are available from the shell via `python -m pypddl --prefix`,
`--cmake-dir`, `--include-dir`, and `--version`.

Downstream CMake projects should include the native prefixes of `pypddl` and
its native package dependencies in `CMAKE_PREFIX_PATH`:

```console
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH="$(python -m pypddl --prefix);$(python -m pyyggdrasil --prefix)"
```

Loki exports the `loki::parsers` target.

## Running the Executables

The executable illustrates how to use Loki. It is disabled by default and can be
enabled with `-DLOKI_BUILD_EXECUTABLES=ON`. Example PDDL inputs come straight from the
[pypddl-datasets](https://pypi.org/project/pypddl-datasets/) cache:

```console
./build/exe/loki $(python -c "import pypddl_datasets as pb; t = pb.fetch_task('classical/tests/gripper/test-1.pddl'); print(t.domain_path, t.task_path)")
```

Use `--out-domain`/`--out-problem` to write the translated PDDL to files, and
see `./build/exe/loki --help` for the parser and translator flags
(`--strict`, `--add-action-costs`, `--compile-typing`,
`--compile-conditional-effects`, `--materialize-equality`,
`--normalize-arithmetic-expressions`).

## Citing Loki

If you use Loki in your research, please cite it as follows:

```bibtex
@misc{drexler-zenodo2026,
  author =       "Dominik Drexler",
  title =        "{Loki}: A {PDDL} Parser and Normalizer",
  publisher =    "Zenodo",
  year =         "2026",
  doi =          "10.5281/zenodo.20081136",
  url =          "https://doi.org/10.5281/zenodo.20081136",
}
```

## Acknowledgements

This work was partially supported by the Wallenberg AI, Autonomous Systems and Software Program (WASP) funded by the Knut and Alice Wallenberg Foundation.
