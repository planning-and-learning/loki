find_package(
    Python 3.11 REQUIRED
    COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule
)

find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
message(STATUS "Found nanobind: ${nanobind_DIR} (found version ${nanobind_VERSION})")

yggdrasil_python_package_init(PYPDDL pypddl)

file(
    GLOB PYPDDL_FORMALISM_SOURCES
    CONFIGURE_DEPENDS
    "src/pypddl/formalism/*.cpp"
)

nanobind_add_module(
    _pypddl
    NB_SUPPRESS_WARNINGS
    src/module.cpp
    ${PYPDDL_FORMALISM_SOURCES}
    BACKEND_MODULE nanobind_backend
)

target_link_libraries(_pypddl PRIVATE loki::parsers)
target_include_directories(_pypddl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

yggdrasil_python_module_rpaths(PYPDDL _pypddl)

install(TARGETS _pypddl DESTINATION "pypddl" COMPONENT pypddl)

set_property(
    TARGET parsers PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
)
yggdrasil_install_native_export(
    PKG_UPPER PYPDDL
    PACKAGE pypddl
    CONFIG_NAME loki
    NAMESPACE loki::
    EXPORT_NAME pypddlLokiparsersTargets
    EXPORT_FILE lokiparsersTargets.cmake
    TARGETS parsers
    HEADER_DIRS "${CMAKE_SOURCE_DIR}/include/loki"
    CONFIG_TEMPLATE "${CMAKE_SOURCE_DIR}/Config.cmake.in"
    VERSION_FILE "${CMAKE_BINARY_DIR}/lokiConfigVersion.cmake"
    CMAKE_SOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake"
)

yggdrasil_install_runtime_path_fixup(PYPDDL pypddl)

set(PYPDDL_BUILD_TREE_PACKAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/pypddl")
file(
    MAKE_DIRECTORY
    "${PYPDDL_BUILD_TREE_PACKAGE_DIR}"
    "${PYPDDL_BUILD_TREE_PACKAGE_DIR}/formalism"
)

set(PYPDDL_GENERATED_VERSION_FILE "${PYPDDL_BUILD_TREE_PACKAGE_DIR}/_version.py")
file(WRITE "${PYPDDL_GENERATED_VERSION_FILE}" "__version__ = \"${PROJECT_VERSION}\"\n")
install(
    FILES "${PYPDDL_GENERATED_VERSION_FILE}"
    DESTINATION "pypddl"
    COMPONENT pypddl
)

yggdrasil_install_python_files(
    pypddl "${CMAKE_CURRENT_SOURCE_DIR}/src"
    FILES
        pypddl/__init__.py
        pypddl/__main__.py
        pypddl/__init__.pyi
        pypddl/py.typed
        pypddl/formalism/__init__.py
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pypddl/__init__.py"
    "${PYPDDL_BUILD_TREE_PACKAGE_DIR}/__init__.py"
    COPYONLY
)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pypddl/__init__.pyi"
    "${PYPDDL_BUILD_TREE_PACKAGE_DIR}/__init__.pyi"
    COPYONLY
)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pypddl/py.typed"
    "${PYPDDL_BUILD_TREE_PACKAGE_DIR}/py.typed"
    COPYONLY
)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pypddl/formalism/__init__.py"
    "${PYPDDL_BUILD_TREE_PACKAGE_DIR}/formalism/__init__.py"
    COPYONLY
)
add_custom_command(
    TARGET _pypddl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:_pypddl>
        "${PYPDDL_BUILD_TREE_PACKAGE_DIR}/$<TARGET_FILE_NAME:_pypddl>"
    VERBATIM
)

if(LOKI_BUILD_TESTS)
    add_test(
        NAME pypddl_build_tree_package
        COMMAND
            "${Python_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/run_build_tree_test.py"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_build_tree_package.py"
    )
    set_tests_properties(
        pypddl_build_tree_package PROPERTIES
        ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR};PYTHONDONTWRITEBYTECODE=1"
    )

    add_test(
        NAME pypddl_bindings
        COMMAND
            "${Python_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/run_build_tree_test.py"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_loki_bindings.py"
    )
    set_tests_properties(
        pypddl_bindings PROPERTIES
        ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR};PYTHONDONTWRITEBYTECODE=1"
    )
endif()

yggdrasil_install_provider_env(
    PYPDDL pypddl
    PREFIXES "${YGGDRASIL_NATIVE_PREFIX}"
)

yggdrasil_install_python_stubs(
    PYPDDL pypddl
    MODULE pypddl._pypddl
    PRIVATE_MODULE _pypddl
    RENAME_PACKAGES pypddl pyyggdrasil
)

add_custom_target(pypddl_wheel DEPENDS _pypddl parsers)
