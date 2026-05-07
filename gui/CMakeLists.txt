cmake_minimum_required(VERSION 3.16)

project(gui VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_LIST_DIR}/assets")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../shared" "${CMAKE_CURRENT_BINARY_DIR}/shared")

find_package(Qt6 REQUIRED COMPONENTS Charts Core Gui SerialPort Widgets)

qt_standard_project_setup()

qt_add_executable(oscilloscope-gui
    ${CMAKE_CURRENT_LIST_DIR}/src/gui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/oscilloscope.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/baud_rate_combo_box.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/baud_rate_combo_box.hpp

    ${CMAKE_CURRENT_LIST_DIR}/src/labeled_dial.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/labeled_dial.hpp
    ${CMAKE_CURRENT_LIST_DIR}/src/vertical_dial.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/vertical_dial.hpp
    ${CMAKE_CURRENT_LIST_DIR}/src/vertical_pos_dial.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/vertical_pos_dial.hpp
    ${CMAKE_CURRENT_LIST_DIR}/src/horizontal_dial.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/horizontal_dial.hpp

    ${CMAKE_CURRENT_LIST_DIR}/src/channel_button.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/channel_button.hpp

    ${CMAKE_CURRENT_LIST_DIR}/src/oscilloscope_widget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/oscilloscope_widget.hpp
    ${CMAKE_CURRENT_LIST_DIR}/assets/oscilloscope.ui

    ${CMAKE_CURRENT_LIST_DIR}/src/serial_connection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/serial_connection.hpp
)

target_include_directories(oscilloscope-gui PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include/
)

target_link_libraries(oscilloscope-gui PRIVATE
    c031-oscilloscope-shared
    Qt6::Charts
    Qt6::Core
    Qt6::Gui
    Qt6::SerialPort
    Qt6::Widgets
)
