
TEMPLATE = app
TARGET = GIFEditor.App
QT += core gui widgets

HEADERS = mainwindow.hpp

SOURCES = main.cpp \
    mainwindow.cpp

RESOURCES = resources.qrc

win32 {
    RC_FILE = gif-editor.rc
}
