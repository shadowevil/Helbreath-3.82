cmake_minimum_required(VERSION 3.21)

add_library(SFMLEngine STATIC
    RendererFactory.cpp
    SFMLWindow.cpp
    SFMLRenderer.cpp
    SFMLTexture.cpp
    SFMLSprite.cpp
    SFMLSpriteFactory.cpp
    SFMLBitmapFont.cpp
    SFMLInput.cpp
    SFMLTextRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Render/ISpriteFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Render/TextLib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Render/ResolutionConfig.cpp
)

target_include_directories(SFMLEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Render
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Entity
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Util
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Net
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Item
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Game
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Shared/Packet
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Client/Includes
    ${CMAKE_CURRENT_SOURCE_DIR}/../Client
)

find_package(SFML COMPONENTS Graphics Window System CONFIG REQUIRED)
target_link_libraries(SFMLEngine PUBLIC SFML::Graphics SFML::Window SFML::System)
