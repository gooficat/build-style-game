cmake_minimum_required(VERSION 3.15)

add_compile_options(-Wall -Wextra -Wpedantic -O3 -ffast-math)

project(buildlikefps VERSION 1.0 LANGUAGES C)

add_executable(
	buildlikefps
	src/g_game_state.c
	src/game.c
	src/r_renderer.c
	src/g_collision.c
)

target_include_directories(buildlikefps PRIVATE include)

target_link_libraries(
	buildlikefps
	SDL3
	SDL3_Image
)

add_custom_command(TARGET buildlikefps POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build complete!"
    COMMAND ${CMAKE_COMMAND} -E time "${CMAKE_CURRENT_BINARY_DIR}/buildlikefps.exe"
)
