cmake_minimum_required(VERSION 3.16)

project(AmongUs VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui"
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
    src/main.cpp

    src/ui/mainwindow.cpp
    src/ui/mainwindow.cpp
    src/ui/mainwindow.ui
    include/ui/mainwindow.h
    include/ui/PlayerItem.h

    src/network/Client.cpp
    src/network/ClientConnection.cpp
    src/network/ClientController.cpp
    src/network/GameController.cpp
    src/network/Message.cpp
    src/network/Server.cpp
    include/network/Client.h
    include/network/ClientConnection.h
    include/network/ClientController.h
    include/network/GameController.h
    include/network/Message.h
    include/network/MessageType.h
    include/network/Server.h

    src/game/ChatMessage.cpp
    src/game/DeadBody.cpp
    src/game/GameMap.cpp
    src/game/GameState.cpp
    src/game/Player.cpp
    src/game/Task.cpp
    src/game/VotingSession.cpp
    src/game/tasks/WireTask.cpp
    include/game/ChatMessage.h
    include/game/DeadBody.h
    include/game/GameMap.h
    include/game/GameState.h
    include/game/GameTypes.h
    include/game/Player.h
    include/game/Task.h
    include/game/VotingSession.h
        include/game/tasks/WireTask.h

    resources.qrc
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(AmongUs
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        include/network/PlayerNetworkSession.h
        src/network/PlayerNetworkSession.cpp
        include/ui/PlayerItem.h
        src/ui/PlayerItem.cpp
    )
else()
    if(ANDROID)
        add_library(AmongUs SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(AmongUs
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(AmongUs PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ui"
)

target_link_libraries(AmongUs PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

if(QT_VERSION VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION
        MACOSX_BUNDLE_GUI_IDENTIFIER com.example.AmongUs
    )
endif()

set_target_properties(AmongUs PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING
        ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
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
endif()