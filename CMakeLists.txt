cmake_minimum_required(VERSION 3.21)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/read_project_version.cmake")
loki_read_project_version(LOKI_PROJECT_VERSION "${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml")

project(loki VERSION "${LOKI_PROJECT_VERSION}" LANGUAGES C CXX)

# Project configuration

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

set(LOKI_INSTALL_INTERFACE_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")

# Options

option(LOKI_USE_LLD "Use LLVM lld with Clang when available." ON)
option(LOKI_ENABLE_LTO "Enable link-time optimization for Release builds." ON)
option(LOKI_BUILD_PYPDDL "Build Loki for the pypddl Python wheel." OFF)
option(LOKI_BUILD_TESTS "Build Loki tests." OFF)
option(LOKI_BUILD_EXECUTABLES "Build Loki executables." OFF)
option(LOKI_BUILD_PROFILING "Build Loki profiling targets." OFF)

# Build configuration

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(
        CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE
    )
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W1 /EHsc /bigobj /MP")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " /IGNORE:4006,4044,4075")
else()
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    add_compile_options(-Wall -Wextra)

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g2 -ggdb")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g2 -fno-omit-frame-pointer -fno-lto")

    if(
        LOKI_USE_LLD
        AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    )
        find_program(LLD_LINKER NAMES ld.lld)
        if(LLD_LINKER)
            message(STATUS "Using lld: ${LLD_LINKER}")
            add_link_options(-fuse-ld=lld)
        else()
            message(STATUS "lld not found, falling back to default linker")
        endif()
    elseif(LOKI_USE_LLD AND LOKI_ENABLE_LTO AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(STATUS "Skipping lld with GNU because GCC LTO requires the GCC linker plugin")
    endif()

    if(LOKI_ENABLE_LTO AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # Cap release LTO parallelism to reduce RAM spikes.
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=2")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto=2")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -flto=2")
        set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -flto=2")
    elseif(
        LOKI_ENABLE_LTO
        AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    )
        # ThinLTO reduces release compile and link costs compared with full LTO.
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=thin")
        if(APPLE)
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,-mllvm,-threads=2")
            set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,-mllvm,-threads=2")
            set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -Wl,-mllvm,-threads=2")
        else()
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--thinlto-jobs=2")
            set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--thinlto-jobs=2")
            set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -Wl,--thinlto-jobs=2")
        endif()
    endif()
endif()

if(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Build configurations: ${CMAKE_CONFIGURATION_TYPES}")
else()
    message(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")
endif()

# Dependencies

set(LOKI_YGGDRASIL_REQUIRED_VERSION 0.3.1)

include("${PROJECT_SOURCE_DIR}/cmake/bootstrap_pyyggdrasil.cmake")

find_package(
    yggdrasil ${LOKI_YGGDRASIL_REQUIRED_VERSION}
    CONFIG REQUIRED
    PATHS ${CMAKE_PREFIX_PATH}
    NO_DEFAULT_PATH
)
if(yggdrasil_FOUND)
    message(STATUS "Found yggdrasil: ${yggdrasil_DIR} (found version ${yggdrasil_VERSION})")
endif()

yggdrasil_register_native_dependency_prefix("${YGGDRASIL_NATIVE_PREFIX}")
yggdrasil_register_python_native_runtime_prefix("pyyggdrasil" "${YGGDRASIL_NATIVE_PREFIX}")

# Find Boost headers only according to https://cmake.org/cmake/help/latest/module/FindBoost.html
find_package(Boost 1.74.0 REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    message(STATUS "Found Boost: ${Boost_DIR} (found version ${Boost_VERSION})")
endif()

find_package(gtl CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(gtl_FOUND)
    message(STATUS "Found gtl: ${gtl_DIR}")
endif()

find_package(fmt CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(fmt_FOUND)
    message(STATUS "Found fmt: ${fmt_DIR} (found version ${fmt_VERSION})")
endif()

# Benchmark setup

if(LOKI_BUILD_TESTS)
    # Benchmark PDDL comes from the pypddl-datasets cache, resolved (and
    # downloaded on demand) at configure time.
    find_package(Python 3.11 REQUIRED COMPONENTS Interpreter)
    execute_process(
        COMMAND
            "${Python_EXECUTABLE}"
            -c
            "import pypddl_datasets, sys; sys.stdout.write(str(pypddl_datasets.data_root()))"
        OUTPUT_VARIABLE LOKI_BENCHMARKS_DIR
        RESULT_VARIABLE LOKI_BENCHMARKS_RESULT
    )
    if(NOT LOKI_BENCHMARKS_RESULT EQUAL 0)
        message(
            FATAL_ERROR
            "Could not resolve the benchmark data root; install 'pypddl-datasets>=0.0.9,<0.1' into the configuring Python (${Python_EXECUTABLE})."
        )
    endif()
    message(STATUS "Benchmark data: ${LOKI_BENCHMARKS_DIR}")
    add_compile_definitions(BENCHMARKS_DIR="${LOKI_BENCHMARKS_DIR}")
endif()

# Targets

if(LOKI_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(src)

if(LOKI_BUILD_PYPDDL)
    add_subdirectory(python)
endif()

if(LOKI_BUILD_EXECUTABLES)
    add_subdirectory(exe)
endif()

if(LOKI_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(LOKI_BUILD_PROFILING)
    add_subdirectory(profiling)
endif()

# Installation and package export

if(NOT LOKI_BUILD_PYPDDL)
    install(
        DIRECTORY "${PROJECT_SOURCE_DIR}/include/loki"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        COMPONENT loki
    )

    install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/loki/cmake"
        COMPONENT loki
    )
endif()

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/lokiConfigVersion.cmake"
    VERSION ${loki_VERSION}
    COMPATIBILITY SameMinorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/lokiConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/loki"
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

if(NOT LOKI_BUILD_PYPDDL)
    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/lokiConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/lokiConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/loki"
        COMPONENT loki
    )
endif()
