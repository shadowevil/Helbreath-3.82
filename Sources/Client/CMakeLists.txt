cmake_minimum_required(VERSION 3.21)
project(HelbreathClient CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SFMLEngine sub-project
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../SFMLEngine ${CMAKE_BINARY_DIR}/SFMLEngine)

# nlohmann-json (header-only, from vcpkg)
find_package(nlohmann_json CONFIG REQUIRED)

# Collect client sources
file(GLOB CLIENT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Remove .bak files
list(FILTER CLIENT_SOURCES EXCLUDE REGEX "\\.bak$")

# Remove .rc files on non-Windows
if(NOT WIN32)
    list(FILTER CLIENT_SOURCES EXCLUDE REGEX "\\.rc$")
endif()

# Add shared dependency sources that need compiling
list(APPEND CLIENT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Net/ASIOSocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Net/IOServicePool.cpp
)

add_executable(HelbreathClient ${CLIENT_SOURCES})

# ASIO_STANDALONE is already defined in ASIOSocket.h, no need to duplicate here

target_include_directories(HelbreathClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Render
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Entity
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Util
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Net
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Item
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Game
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Packet
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/ASIO
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Client/Includes
    ${CMAKE_CURRENT_SOURCE_DIR}/../SFMLEngine
)

target_link_libraries(HelbreathClient PRIVATE
    SFMLEngine
    nlohmann_json::nlohmann_json
)

# Linux-specific linking
if(UNIX AND NOT APPLE)
    target_link_libraries(HelbreathClient PRIVATE
        pthread
        dl
        m
    )
endif()

# Output directory
set_target_properties(HelbreathClient PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)
