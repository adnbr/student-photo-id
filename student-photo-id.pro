#-------------------------------------------------
#
# Project created by QtCreator 2015-11-02T15:01:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

TARGET = stu-pid
INCLUDEPATH += /usr/local/include/opencv /usr/include/tesseract
LIBS += -L/usr/local/lib -lopencv_core -lopencv_highgui -lopencv_imgproc -ltesseract


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
