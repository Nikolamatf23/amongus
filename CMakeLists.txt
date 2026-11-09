cmake_minimum_required(VERSION 3.16)

project(AmongUs VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(CORE_SOURCES
    src/network/Client.cpp
    src/network/ClientConnection.cpp
    src/network/ClientController.cpp
    src/network/GameController.cpp
    src/network/Message.cpp
    src/network/Server.cpp
    src/network/PlayerNetworkSession.cpp

    src/game/ChatMessage.cpp
    src/game/DeadBody.cpp
    src/game/GameMap.cpp
    src/game/GameState.cpp
    src/game/Player.cpp
    src/game/Task.cpp
    src/game/VotingSession.cpp
    src/game/tasks/WireTask.cpp
)

set(CORE_HEADERS
    include/network/Client.h
    include/network/ClientConnection.h
    include/network/ClientController.h
    include/network/GameController.h
    include/network/Message.h
    include/network/MessageType.h
    include/network/Server.h
    include/network/PlayerNetworkSession.h

    include/game/ChatMessage.h
    include/game/DeadBody.h
    include/game/GameMap.h
    include/game/GameState.h
    include/game/GameTypes.h
    include/game/Player.h
    include/game/Task.h
    include/game/VotingSession.h
    include/game/tasks/WireTask.h
)

set(UI_SOURCES
    src/ui/mainwindow.cpp
    src/ui/StartScreen.cpp
    src/ui/LobbyScreen.cpp
    src/ui/ChatWidget.cpp
    src/ui/VotingScreen.cpp
    src/ui/GameOverScreen.cpp
    src/ui/GameScreen.cpp
    src/ui/PlayerItem.cpp
    src/ui/TaskDialog.cpp
    src/ui/EmergencyDialog.cpp
)

set(UI_HEADERS
    include/ui/mainwindow.h
    include/ui/StartScreen.h
    include/ui/LobbyScreen.h
    include/ui/ChatWidget.h
    include/ui/VotingScreen.h
    include/ui/GameOverScreen.h
    include/ui/GameScreen.h
    include/ui/PlayerItem.h
    include/ui/TaskDialog.h
    include/ui/EmergencyDialog.h
)

set(COMMON_RESOURCES
    resources.qrc
)

set(PROJECT_SOURCES
    src/main.cpp
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${UI_SOURCES}
    ${UI_HEADERS}
    ${COMMON_RESOURCES}
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(AmongUs MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    if(ANDROID)
        add_library(AmongUs SHARED ${PROJECT_SOURCES})
    else()
        add_executable(AmongUs ${PROJECT_SOURCES})
    endif()
endif()

target_include_directories(AmongUs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/network
    ${CMAKE_CURRENT_SOURCE_DIR}/include/game
)

target_link_libraries(AmongUs PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

# Samostalni test GameScreen-a. Test se ne dodaje u glavni AmongUs program.
set(GAME_SCREEN_TEST_SOURCES
    src/game_screen_test.cpp
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    src/ui/GameScreen.cpp
    src/ui/PlayerItem.cpp
    src/ui/TaskDialog.cpp
    src/ui/EmergencyDialog.cpp
    include/ui/GameScreen.h
    include/ui/PlayerItem.h
    include/ui/TaskDialog.h
    include/ui/EmergencyDialog.h
    ${COMMON_RESOURCES}
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(GameScreenTest MANUAL_FINALIZATION
        ${GAME_SCREEN_TEST_SOURCES})
else()
    add_executable(GameScreenTest ${GAME_SCREEN_TEST_SOURCES})
endif()

target_include_directories(GameScreenTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/network
    ${CMAKE_CURRENT_SOURCE_DIR}/include/game
)

target_link_libraries(GameScreenTest PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

set_target_properties(AmongUs PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS AmongUs
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AmongUs)
    qt_finalize_executable(GameScreenTest)
endif()
