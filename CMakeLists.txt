cmake_minimum_required(VERSION 3.16)

project(GestionSalles VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

add_executable(GestionSalles
    main.cpp
    theme.h theme.cpp
    database.h database.cpp
    widgets.h widgets.cpp
    formdialog.h formdialog.cpp
    basepage.h basepage.cpp
    homepage.h homepage.cpp
    sallepage.h sallepage.cpp
    enseignantpage.h enseignantpage.cpp
    matierepage.h matierepage.cpp
    reservationpage.h reservationpage.cpp
    planningpage.h planningpage.cpp
    mainwindow.h mainwindow.cpp
)

target_link_libraries(GestionSalles PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

set_target_properties(GestionSalles PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

include(GNUInstallDirs)
install(TARGETS GestionSalles
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
