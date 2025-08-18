cmake_minimum_required(VERSION 3.10)
project(ltspice_simulator_cpp_sdl2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.7.3/mingw_64")
find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

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
)

# Build executable
add_executable(LTspice ${SOURCES})

target_include_directories(LTspice PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/eigen-3.4.0
)

target_link_libraries(LTspice PRIVATE Qt6::Widgets Qt6::Charts)