
TEMPLATE = app
TARGET = GIFEditor.App
QT += core gui widgets

HEADERS = mainwindow.hpp \
	frame.hpp

SOURCES = main.cpp \
    mainwindow.cpp \
	frame.cpp

RESOURCES = resources.qrc

win32 {
    RC_FILE = gif-editor.rc
}
