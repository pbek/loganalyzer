cmake_minimum_required(VERSION 3.2.2)
project(LogAnalyzer)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package( Qt5Core 5.3 REQUIRED )
find_package( Qt5Widgets REQUIRED )
find_package( Qt5Gui REQUIRED )
find_package( Qt5Svg REQUIRED )

set(RESOURCE_FILES
    media.qrc
)

qt5_add_resources(RESOURCE_ADDED ${RESOURCE_FILES})

set(SOURCE_FILES
    version.h
    release.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    qtexteditsearchwidget.cpp
    qtexteditsearchwidget.h
)

add_executable(LogAnalyzer ${SOURCE_FILES} ${RESOURCE_ADDED})


# The Qt5Widgets_INCLUDES also includes the include directories for
# dependencies QtCore and QtGui
include_directories(${Qt5Widgets_INCLUDES} ${Qt5Svg_INCLUDES})

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS} ${Qt5Svg_DEFINITIONS})

# Executables fail to build with Qt 5 in the default configuration
# without -fPIE. We add that here.
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${Qt5Svg_EXECUTABLE_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "-std=c++0x")

# The Qt5Widgets_LIBRARIES variable also includes QtGui and QtCore
target_link_libraries(LogAnalyzer ${Qt5Widgets_LIBRARIES} ${Qt5Svg_LIBRARIES})
