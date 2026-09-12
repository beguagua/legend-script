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
  src/opengl_backend.cpp
)

option(LEGEND_ENABLE_OPENGL "Enable the OpenGL graphics backend contract" ON)
if(LEGEND_ENABLE_OPENGL)
  find_package(OpenGL QUIET)
  if(OpenGL_FOUND)
    target_compile_definitions(legend PRIVATE LEGEND_HAS_OPENGL=1)
    target_link_libraries(legend PRIVATE OpenGL::GL)
  else()
    message(STATUS "OpenGL SDK not found; keeping the portable OpenGL backend contract")
  endif()
endif()

target_compile_options(legend PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

install(TARGETS legend DESTINATION bin)

include(CTest)
if(BUILD_TESTING)
  add_test(NAME legend_smoke COMMAND legend -e "output(2 + 3 * 4)")
  set_tests_properties(legend_smoke PROPERTIES PASS_REGULAR_EXPRESSION "14")
  add_test(NAME legend_game_runtime COMMAND legend --run ${CMAKE_SOURCE_DIR}/examples/complete_game.lgnd)
  set_tests_properties(legend_game_runtime PROPERTIES PASS_REGULAR_EXPRESSION "scene=arena")
  add_test(NAME legend_opengl_versions COMMAND legend --opengl-version 4.6)
  set_tests_properties(legend_opengl_versions PROPERTIES PASS_REGULAR_EXPRESSION "OpenGL 4.6")
endif()

add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND} -E echo "Remove installed files listed in install_manifest.txt"
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_INSTALL_PREFIX}/bin"
  COMMENT "Uninstall Legend Script"
)

install(FILES README.md LICENSE DESTINATION share/doc/legend-script)
install(DIRECTORY examples/ DESTINATION share/legend-script/examples)
