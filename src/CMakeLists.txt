add_library(
    parsers SHARED
    formalism/formatter.cpp
    formalism/repository.cpp
    parser.cpp
    semantic/builder.cpp
    semantic/builder_declarations.cpp
    semantic/builder_effects.cpp
    semantic/builder_expressions.cpp
    semantic/canonical_copy.cpp
    semantic/canonical_copy_expressions.cpp
    semantic/canonical_copy_symbols.cpp
    semantic/context.cpp
    semantic/diagnostics.cpp
    semantic/infer_requirements.cpp
    semantic/mappings.cpp
    semantic/parser.cpp
    semantic/translation_storage.cpp
    semantic/translator.cpp
)
add_library(loki::parsers ALIAS parsers)
set_target_properties(parsers PROPERTIES OUTPUT_NAME loki_parsers)

target_compile_features(parsers PUBLIC cxx_std_20)

target_compile_definitions(
    parsers
    PUBLIC
        BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
        BOOST_MPL_LIMIT_LIST_SIZE=50
)

target_include_directories(
    parsers
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${YGGDRASIL_NATIVE_DEPENDENCY_INCLUDE_DIRECTORIES}>"
        "$<INSTALL_INTERFACE:${LOKI_INSTALL_INTERFACE_INCLUDEDIR}>"
)

target_link_libraries(
    parsers
    PUBLIC
        yggdrasil::yggdrasil
        gtl::gtl
        fmt::fmt
)

if(APPLE)
    set(LOKI_LIBRARY_ORIGIN "@loader_path")
elseif(UNIX)
    set(LOKI_LIBRARY_ORIGIN "$ORIGIN")
endif()

foreach(prefix IN LISTS CMAKE_PREFIX_PATH)
    if(EXISTS "${prefix}/${CMAKE_INSTALL_LIBDIR}")
        list(APPEND LOKI_PARSERS_BUILD_RPATHS "${prefix}/${CMAKE_INSTALL_LIBDIR}")
    endif()
endforeach()

set_target_properties(
    parsers
    PROPERTIES
        BUILD_RPATH "${LOKI_PARSERS_BUILD_RPATHS}"
        INSTALL_RPATH "${LOKI_LIBRARY_ORIGIN}"
)

# Installation and package export

if(NOT LOKI_BUILD_PYPDDL)
    install(
        TARGETS parsers
        EXPORT lokiparsersTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT loki
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT loki
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT loki
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        EXPORT lokiparsersTargets
        NAMESPACE loki::
        FILE "lokiparsersTargets.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/loki"
        COMPONENT loki
    )

    export(
        EXPORT lokiparsersTargets
        FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/lokiparsersTargets.cmake"
        NAMESPACE loki::
    )
endif()
