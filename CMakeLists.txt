cmake_minimum_required(VERSION 3.16)

project(AmongUs VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui")

# Iskljucivo Qt6. Qt Creator ga pronalazi preko izabranog Qt6 kita.
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(PROJECT_SOURCES
        src/main.cpp

        src/ui/mainwindow.cpp
        src/ui/mainwindow.ui
        include/ui/mainwindow.h

        src/network/Client.cpp
        src/network/ClientConnection.cpp
        src/network/ClientController.cpp
        src/network/GameController.cpp
        src/network/Message.cpp
        src/network/PlayerNetworkSession.cpp
        src/network/Server.cpp
        include/network/Client.h
        include/network/ClientConnection.h
        include/network/ClientController.h
        include/network/GameController.h
        include/network/Message.h
        include/network/MessageType.h
        include/network/PlayerNetworkSession.h
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
)

# src/game/Message.cpp i include/game/Message.h su stara, duplirana
# implementacija klase Message. Aktivna verzija je u network/.
# Nemoj ukljucivati obe implementacije u isti izvrsni fajl.

qt_add_executable(AmongUs
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
)

target_include_directories(AmongUs PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/ui"
)

target_link_libraries(AmongUs PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
)

if(MSVC)
    target_compile_options(AmongUs PRIVATE /utf-8)
endif()

set_target_properties(AmongUs PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.amongus.game"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
)

include(GNUInstallDirs)
install(TARGETS AmongUs
        BUNDLE DESTINATION .
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

qt_finalize_executable(AmongUs)
