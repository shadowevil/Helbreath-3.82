cmake_minimum_required(VERSION 3.21)
project(HelbreathServer LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# --------------------------------------------------------------------------
# SQLite (vendored amalgamation)
# --------------------------------------------------------------------------
add_library(sqlite3 STATIC
    ../Dependencies/SQLite/sqlite3.c
)
target_include_directories(sqlite3 PUBLIC
    ../Dependencies/SQLite
    ../Dependencies/Server/Includes
)
if(UNIX)
    target_link_libraries(sqlite3 PUBLIC dl)
endif()
# Suppress warnings in vendored code
if(MSVC)
    target_compile_options(sqlite3 PRIVATE /W0)
else()
    target_compile_options(sqlite3 PRIVATE -w)
endif()

# --------------------------------------------------------------------------
# OpenSSL (via vcpkg or system)
# --------------------------------------------------------------------------
find_package(OpenSSL REQUIRED)

# --------------------------------------------------------------------------
# Server sources
# --------------------------------------------------------------------------
set(SERVER_SOURCES
    AccountSqliteStore.cpp
    ChatLog.cpp
    Client.cpp
    CmdBroadcast.cpp
    CmdGiveItem.cpp
    CmdHelp.cpp
    CmdReload.cpp
    CmdSetAdmin.cpp
    CmdSetCmdLevel.cpp
    CmdShowChat.cpp
    CombatManager.cpp
    CommonTypes.cpp
    CraftingManager.cpp
    DelayEventManager.cpp
    DynamicObjectManager.cpp
    EntityManager.cpp
    FishingManager.cpp
    Game.cpp
    GameChatCommand.cpp
    GameCmdBlock.cpp
    GameCmdCome.cpp
    GameCmdCreateItem.cpp
    GameCmdGiveItem.cpp
    GameCmdGM.cpp
    GameCmdGoto.cpp
    GameCmdInvis.cpp
    GameCmdRegen.cpp
    GameCmdSpawn.cpp
    GameCmdUnblock.cpp
    GameCmdWhisper.cpp
    GameConfigSqliteStore.cpp
    GuildManager.cpp
    ItemLog.cpp
    ItemManager.cpp
    LoginServer.cpp
    LootManager.cpp
    MagicManager.cpp
    Map.cpp
    MapInfoSqliteStore.cpp
    MiningManager.cpp
    NetworkMsg.cpp
    Npc.cpp
    PartyManager.cpp
    Quest.cpp
    QuestManager.cpp
    ServerCommand.cpp
    ServerConsole.cpp
    SkillManager.cpp
    StatusEffectManager.cpp
    Tile.cpp
    WarManager.cpp
    Wmain.cpp
)

# Shared sources (ASIO networking)
set(SHARED_SOURCES
    ../Dependencies/Shared/Net/ASIOSocket.cpp
    ../Dependencies/Shared/Net/IOServicePool.cpp
)

# --------------------------------------------------------------------------
# Executable
# --------------------------------------------------------------------------
if(WIN32)
    add_executable(HelbreathServer WIN32 ${SERVER_SOURCES} ${SHARED_SOURCES})
else()
    add_executable(HelbreathServer ${SERVER_SOURCES} ${SHARED_SOURCES})
endif()

# --------------------------------------------------------------------------
# Include paths (mirror vcxproj)
# --------------------------------------------------------------------------
target_include_directories(HelbreathServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../Dependencies/Shared
    ../Dependencies/Shared/ASIO
    ../Dependencies/Shared/Entity
    ../Dependencies/Shared/Game
    ../Dependencies/Shared/Net
    ../Dependencies/Shared/Render
    ../Dependencies/Shared/Util
    ../Dependencies/Shared/Item
    ../Dependencies/Shared/Packet
    ../Dependencies/Server/Includes
)

# --------------------------------------------------------------------------
# Link libraries
# --------------------------------------------------------------------------
target_link_libraries(HelbreathServer PRIVATE
    sqlite3
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(WIN32)
    target_link_libraries(HelbreathServer PRIVATE ws2_32 winmm)
else()
    target_link_libraries(HelbreathServer PRIVATE pthread)
endif()

# --------------------------------------------------------------------------
# Platform-specific compile options
# --------------------------------------------------------------------------
if(MSVC)
    target_compile_options(HelbreathServer PRIVATE /W3 /MP)
    target_compile_definitions(HelbreathServer PRIVATE
        WIN32
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
    )
    # Release optimizations
    target_compile_options(HelbreathServer PRIVATE
        $<$<CONFIG:Release>:/O2 /Ob2 /Oi /GL>
    )
    target_link_options(HelbreathServer PRIVATE
        $<$<CONFIG:Release>:/LTCG>
    )
else()
    target_compile_options(HelbreathServer PRIVATE
        -Wall
        -Wno-unused-result
        -Wno-write-strings
        -fpermissive
    )
endif()
