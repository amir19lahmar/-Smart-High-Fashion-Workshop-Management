cmake_minimum_required(VERSION 3.16)
project(GestionMachines LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Svg)

add_executable(GestionMachines
    src/main.cpp
    src/mainwindow.cpp     src/mainwindow.h     src/mainwindow.ui
    src/machinespage.cpp   src/machinespage.h   src/machinespage.ui
    src/sidebarwidget.cpp  src/sidebarwidget.h
    src/detailsdialog.cpp  src/detailsdialog.h  src/detailsdialog.ui
    src/database.cpp       src/database.h
    src/uihelpers.cpp      src/uihelpers.h
    src/chartwidgets.cpp   src/chartwidgets.h
    src/bannerwidget.cpp   src/bannerwidget.h
    src/machine.h
    resources.qrc
)

target_include_directories(GestionMachines PRIVATE src)

if(MSVC)
    target_compile_options(GestionMachines PRIVATE /utf-8)
endif()

target_link_libraries(GestionMachines PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Svg)
