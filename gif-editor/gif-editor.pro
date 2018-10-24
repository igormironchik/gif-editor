
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

unix: LIBS += -lMagick++-6.Q16 -lMagickCore-6.Q16
win32: LIBS += -lCORE_RL_Magick++_ -lCORE_RL_magick_ -lCORE_RL_croco_ -ladvapi32 -luser32 -lgdi32 -loleaut32 -lole32

INCLUDEPATH += /usr/include/ImageMagick-6 \
	/usr/include/x86_64-linux-gnu/ImageMagick-6
