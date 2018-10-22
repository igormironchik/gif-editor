
TEMPLATE = app
TARGET = GIFEditor.App
QT += core gui widgets

HEADERS = mainwindow.hpp \
	frame.hpp \
	frameontape.hpp \
	tape.hpp

SOURCES = main.cpp \
    mainwindow.cpp \
	frame.cpp \
	frameontape.cpp \
	tape.cpp

RESOURCES = resources.qrc

win32 {
    RC_FILE = gif-editor.rc
}
