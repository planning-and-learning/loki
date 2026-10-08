find_package(GTest "1.11.0" REQUIRED COMPONENTS GTest PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
find_library(Boost_JSON_LIBRARY boost_json PATHS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES lib NO_DEFAULT_PATH REQUIRED)

function(add_gtest test_name)
    add_executable(${test_name} ${ARGN} main.cpp)
    target_compile_definitions(${test_name} PRIVATE ROOT_DIR="${CMAKE_SOURCE_DIR}")
    target_link_libraries(
        ${test_name}
        PRIVATE
            loki::parsers
            GTest::gtest
            ${Boost_JSON_LIBRARY}
    )

    set(LOKI_TEST_BUILD_RPATH)
    if(YGGDRASIL_NATIVE_PREFIX)
        list(APPEND LOKI_TEST_BUILD_RPATH "${YGGDRASIL_NATIVE_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
    endif()
    if(LOKI_TEST_BUILD_RPATH)
        set_target_properties(${test_name} PROPERTIES BUILD_RPATH "${LOKI_TEST_BUILD_RPATH}")
    endif()

    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES FAIL_REGULAR_EXPRESSION "does NOT link in any test case;Running 0 tests")
endfunction()

add_gtest(parser_counts_suite "parser/parser_counts_suite.cpp")
add_gtest(public_facade "public_facade.cpp")

file(
    GLOB_RECURSE LOKI_DIRECT_PUBLIC_HEADERS
    CONFIGURE_DEPENDS
    RELATIVE "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/include/loki/*.hpp"
)
list(SORT LOKI_DIRECT_PUBLIC_HEADERS)

set(LOKI_PUBLIC_HEADER_SOURCES)
foreach(header IN LISTS LOKI_DIRECT_PUBLIC_HEADERS)
    string(REGEX REPLACE "[^A-Za-z0-9_]+" "_" header_target_name "${header}")
    set(header_test_source "${CMAKE_CURRENT_BINARY_DIR}/public_header_${header_target_name}.cpp")
    file(
        GENERATE
        OUTPUT "${header_test_source}"
        CONTENT "#include <${header}>
"
    )
    list(APPEND LOKI_PUBLIC_HEADER_SOURCES "${header_test_source}")
endforeach()
if(LOKI_PUBLIC_HEADER_SOURCES)
    add_library(public_header_compile_checks OBJECT ${LOKI_PUBLIC_HEADER_SOURCES})
    target_link_libraries(public_header_compile_checks PRIVATE loki::parsers)
endif()
add_gtest(semantic_parser_tests "semantic/parser_tests.cpp")
add_gtest(semantic_translator_tests "semantic/translator_tests.cpp")
add_gtest(semantic_diagnostics_tests "semantic/diagnostics_tests.cpp")
add_gtest(semantic_action_costs_tests "semantic/action_costs_tests.cpp")
add_gtest(semantic_negative_suite "semantic/negative_suite.cpp")
add_gtest(semantic_invariant_suite "semantic/invariant_suite.cpp")
add_gtest(semantic_translation_counts_suite "semantic/translation_counts_suite.cpp")
add_gtest(formalism_translator_tests "formalism/translator_tests.cpp")
add_gtest(
    formalism_repository_types
    "formalism/action.cpp"
    "formalism/atom.cpp"
    "formalism/axiom.cpp"
    "formalism/binary_function_expression.cpp"
    "formalism/condition.cpp"
    "formalism/condition_and.cpp"
    "formalism/condition_exists.cpp"
    "formalism/condition_forall.cpp"
    "formalism/condition_imply.cpp"
    "formalism/condition_literal.cpp"
    "formalism/condition_not.cpp"
    "formalism/condition_numeric_constraint.cpp"
    "formalism/condition_or.cpp"
    "formalism/domain.cpp"
    "formalism/effect.cpp"
    "formalism/effect_and.cpp"
    "formalism/effect_forall.cpp"
    "formalism/effect_literal.cpp"
    "formalism/effect_numeric.cpp"
    "formalism/effect_one_of.cpp"
    "formalism/effect_probabilistic.cpp"
    "formalism/effect_probabilistic_alternative.cpp"
    "formalism/effect_when.cpp"
    "formalism/function_expression.cpp"
    "formalism/function_expression_number.cpp"
    "formalism/function_skeleton.cpp"
    "formalism/function_term.cpp"
    "formalism/initial_function_value.cpp"
    "formalism/literal.cpp"
    "formalism/metric.cpp"
    "formalism/multi_function_expression.cpp"
    "formalism/object.cpp"
    "formalism/parameter.cpp"
    "formalism/predicate.cpp"
    "formalism/requirement.cpp"
    "formalism/task.cpp"
    "formalism/term.cpp"
    "formalism/type.cpp"
    "formalism/unary_function_expression.cpp"
    "formalism/variable.cpp"
)
