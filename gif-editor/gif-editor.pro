
TEMPLATE = app
TARGET = GIFEditor.App
QT += core gui widgets

HEADERS = mainwindow.hpp \
	frame.hpp \
	frameontape.hpp \
	tape.hpp \
	view.hpp

SOURCES = main.cpp \
    mainwindow.cpp \
	frame.cpp \
	frameontape.cpp \
	tape.cpp \
	view.cpp

RESOURCES = resources.qrc

win32 {
    RC_FILE = gif-editor.rc
}

unix|win32: LIBS += -L/usr/lib/x86_64-linux-gnu/ -lMagick++-6.Q16 -lMagickCore-6.Q16

INCLUDEPATH += /usr/include/ImageMagick-6 \
	/usr/include/x86_64-linux-gnu/ImageMagick-6
