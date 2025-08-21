cmake_minimum_required(VERSION 3.10)
project(ltspice_simulator_cpp_qt6)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/mingw_64/lib/cmake")

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts Network)  # <-- add Network

# Include Eigen
include_directories("C:/Users/USER/CLionProjects/proj_terminal/eigen-3.4.0")

# Source files
set(SOURCES
        main.cpp
        Component.cpp Component.h
        Circuit.cpp Circuit.h
        ComponentFactory.cpp ComponentFactory.h
        SchematicWidget.cpp SchematicWidget.h
        mainwindow.cpp mainwindow.h
        Dialogs.cpp Dialogs.h
        PlotWindow.cpp PlotWindow.h
        resources.qrc
)

# Build executable
add_executable(ltspice_simulator_cpp_qt6 ${SOURCES})

target_link_libraries(ltspice_simulator_cpp_qt6
        Qt6::Widgets
        Qt6::Charts
        Qt6::Network   # <-- add Network
)

# Automatically run windeployqt after build
add_custom_command(TARGET ltspice_simulator_cpp_qt6 POST_BUILD
        COMMAND "C:\\Qt\\6.9.1\\mingw_64\\bin\\windeployqt.exe" "$<TARGET_FILE:ltspice_simulator_cpp_qt6>"
        COMMENT "Running windeployqt to deploy Qt dependencies..."
)