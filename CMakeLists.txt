cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(deepbot VERSION 1.0.1 LANGUAGES CXX)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK environment variable is not set")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

set(SOURCES
    src/main.cpp
    src/DeepBot.cpp
    src/ui/DeepBotUI.cpp
    src/player/MacroPlayer.cpp
    src/recorder/InputRecorder.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME} geode-sdk)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/formats
    src/ui
    src/recorder
    src/player
    src/utils
)

if(ANDROID)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEEPBOT_ANDROID)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
    )
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEEPBOT_WINDOWS)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Create geode mod package
setup_geode_mod(${PROJECT_NAME})
