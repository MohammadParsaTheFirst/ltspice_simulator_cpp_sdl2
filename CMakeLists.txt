cmake_minimum_required (VERSION 3.29.6)
project (ltspice_simulator_cpp_sdl2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "C:\\Qt\\6.7.3\\mingw_64")

include_directories(eigen-3.4.0)
include_directories(${CMAKE_SOURCE_DIR}/cereal/include)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable (LTspice
        Circuit.cpp
        Circuit.h
        Component.h
        Component.cpp
        WaveForm.cpp
        WaveForm.h
        ComponentFactory.cpp
        ComponentFactory.h
        mainwindow.h
        mainwindow.cpp
        mainwindow.ui
        main.cpp
        ChartWindow.cpp
        ChartWindow.h
        resources.qrc
        SchematicWidget.cpp
        SchematicWidget.h
        Dialogs.cpp
        Dialogs.h
)
target_link_libraries(LTspice Qt6::Widgets Qt6::Charts)