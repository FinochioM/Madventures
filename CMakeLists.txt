cmake_minimum_required(VERSION 3.10)
project(Madventures)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directory for the binary
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add lib directory to include paths for JSON and ImGui
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Game source files
set(GAME_SOURCES
        src/main.cpp
        src/game.cpp
        src/renderer.cpp
        src/entity.cpp
        src/player.cpp
        src/tilemap.cpp
        src/tile.cpp
        src/map_editor.cpp
)

# Check if combat_manager.cpp and enemy.cpp exist (they're in your bat file but not in source docs)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/combat_manager.cpp")
    list(APPEND GAME_SOURCES src/combat_manager.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/enemy.cpp")
    list(APPEND GAME_SOURCES src/enemy.cpp)
endif()

# ImGui library sources - using the full path you provided
set(IMGUI_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui_demo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_sdl2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_sdlrenderer2.cpp
)

# Find SDL2 packages
# Using the exact paths you provided
set(SDL2_DIR "C:/libs/SDL2" CACHE PATH "Path to SDL2")

# Include directories for SDL2
include_directories(
        ${SDL2_DIR}/include
)

# Library directories
link_directories(${SDL2_DIR}/lib)

# Define SDL2 libraries
set(SDL2_LIBRARIES SDL2main SDL2 SDL2_image SDL2_ttf)

# Create executable
add_executable(${PROJECT_NAME} ${GAME_SOURCES} ${IMGUI_SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})

# Set platform-specific properties for Windows
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

# Copy SDL2 DLLs to output directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_DIR}/bin/SDL2.dll"
        "${SDL2_DIR}/bin/SDL2_image.dll"
        "${SDL2_DIR}/bin/SDL2_ttf.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# If using OpenGL
find_package(OpenGL)
if(OPENGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
endif()

# Copy assets to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

# Create maps directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/maps
)

# Print status message
message(STATUS "Configured ${PROJECT_NAME}")