
project( GIFEditor.App )

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)

set( SRC main.cpp
	about.cpp
	busyindicator.cpp
	crop.cpp
	frame.cpp
	frameontape.cpp
	mainwindow.cpp
	tape.cpp
	view.cpp
	about.hpp
	busyindicator.hpp
	crop.hpp
	frame.hpp
	frameontape.hpp
	mainwindow.hpp
	tape.hpp
	view.hpp )

qt5_add_resources( SRC resources.qrc )

if( WIN32 )
	list( APPEND SRC gif-editor.rc )
endif()

include_directories( ${ImageMagick_INCLUDE_DIRS} )

add_executable( GIFEditor.App ${SRC} )

target_link_libraries( GIFEditor.App ${ImageMagick_LIBRARIES} Qt5::Widgets Qt5::Gui Qt5::Core )
