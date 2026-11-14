find_package(benchmark "1.8.3" REQUIRED benchmark Main PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(benchmark)

add_executable(construct_atoms "formalism/construct_atoms.cpp" "support/utils.cpp" "support/utils.hpp")
target_link_libraries(construct_atoms loki::parsers)
target_link_libraries(construct_atoms benchmark::benchmark)
target_include_directories(construct_atoms PRIVATE support)

add_executable(iterate_atoms "formalism/iterate_atoms.cpp" "support/utils.cpp" "support/utils.hpp")
target_link_libraries(iterate_atoms loki::parsers)
target_link_libraries(iterate_atoms benchmark::benchmark)
target_include_directories(iterate_atoms PRIVATE support)
