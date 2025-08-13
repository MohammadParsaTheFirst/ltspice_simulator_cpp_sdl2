#cmake_minimum_required(VERSION 3.30)
#project(proj_terminal)
#set(CMAKE_CXX_STANDARD 20)
#include_directories("C:\\Users\\USER\\CLionProjects\\proj_terminal\\eigen-3.4.0")
#add_executable(proj_terminal main.cpp
#        Component.h
#        Circuit.h
#        Circuit.cpp
#        Component.cpp
#        Waveform.cpp
#        Waveform.h
#        ChartWindow.cpp
#        ChartWindow.h
#        ComponentFactory.cpp
#        ComponentFactory.h
#        SchematicWidget.cpp
#        SchematicWidget.h
#        main_cli.cpp
#        mainwindow.cpp
#        mainwindow.h
#)



cmake_minimum_required(VERSION 3.30)
project(proj_terminal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Tell CMake where to find Qt
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/mingw_64/lib/cmake")

# Find Qt components
find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

# Include Eigen
include_directories("C:/Users/USER/CLionProjects/proj_terminal/eigen-3.4.0")

# Source files
set(SOURCES
        main.cpp
        Component.cpp Component.h
        Circuit.cpp Circuit.h
        Waveform.cpp Waveform.h
        ChartWindow.cpp ChartWindow.h
        ComponentFactory.cpp ComponentFactory.h
        SchematicWidget.cpp SchematicWidget.h
        mainwindow.cpp mainwindow.h
        Dialogs.cpp
        Dialogs.h
)

# Build executable
add_executable(proj_terminal ${SOURCES})

# Link against Qt libraries
target_link_libraries(proj_terminal
        Qt6::Widgets
        Qt6::Charts
)

# Automatically run windeployqt after build
add_custom_command(TARGET proj_terminal POST_BUILD
        COMMAND "C:\\Qt\\6.9.1\\mingw_64\\bin\\windeployqt.exe" "$<TARGET_FILE:proj_terminal>"
        COMMENT "Running windeployqt to deploy Qt dependencies..."
)
#add_custom_command(TARGET proj_terminal POST_BUILD
#        COMMAND "${CMAKE_PREFIX_PATH}/../../../bin/windeployqt.exe" "$<TARGET_FILE:proj_terminal>"
#        COMMENT "Running windeployqt to deploy Qt dependencies..."
#)
# "C:\Qt\6.9.1\mingw_64\bin\windeployqt.exe"