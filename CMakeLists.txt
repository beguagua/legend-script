cmake_minimum_required(VERSION 3.25)

project(LegendScript
  VERSION 0.1.0
  DESCRIPTION "Legend Script: a game-oriented scripting language"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(legend
  src/main.cpp
)

target_compile_options(legend PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

install(TARGETS legend DESTINATION bin)

include(CTest)
if(BUILD_TESTING)
  add_test(NAME legend_smoke COMMAND legend -e "output(2 + 3 * 4)")
  set_tests_properties(legend_smoke PROPERTIES PASS_REGULAR_EXPRESSION "14")
endif()

add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND} -E echo "Remove installed files listed in install_manifest.txt"
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_INSTALL_PREFIX}/bin"
  COMMENT "Uninstall Legend Script"
)

install(FILES README.md LICENSE DESTINATION share/doc/legend-script)
install(DIRECTORY examples/ DESTINATION share/legend-script/examples)
