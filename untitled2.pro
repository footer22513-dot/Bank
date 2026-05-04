QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    adminwidget.cpp \
    bankrecord.cpp \
    barchartwidget.cpp \
    creditcalc.cpp \
    creditformwidget.cpp \
    credittablewidget.cpp \
    datahandler.cpp \
    depositcalc.cpp \
    depositformwidget.cpp \
    depositwidget.cpp \
    linechartwidget.cpp \
    loginwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    successwidget.cpp \
    userwidget.cpp \
    piechartwidget.cpp

HEADERS += \
    adminwidget.h \
    bankrecord.h \
    barchartwidget.h \
    basewidget.h \
    constants.h \
    creditcalc.h \
    creditformwidget.h \
    credittablewidget.h \
    datahandler.h \
    depositcalc.h \
    depositformwidget.h \
    depositwidget.h \
    linechartwidget.h \
    loginwidget.h \
    mainwindow.h \
    successwidget.h \
    userwidget.h \
    piechartwidget.h

FORMS += \
    UserForm.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
