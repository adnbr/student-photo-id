#-------------------------------------------------
#
# Project created by QtCreator 2015-11-02T15:01:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

#ifdef Q_OS_WIN

TARGET = stu-pid.exe
INCLUDEPATH += Y:\opencv\build\include Y:\tesseract-ocr\tesseract Y:\tesseract-ocr\ Y:\tesseract-ocr\ccutil
LIBS += -L"Y:\opencv\build\x86\vc12\lib" -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -ltesseract

#endif

#ifdef Q_OS_LINUX
TARGET = stu-pid
INCLUDEPATH += /usr/local/include/opencv /usr/include/tesseract
LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -ltesseract

#endif

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
