#-------------------------------------------------
#
# Project created by QtCreator 2017-05-27T14:11:55
#
#-------------------------------------------------

QT       += core gui network sql concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Daqt
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += src/Main.cpp\
        src/MainWindow.cpp \
    src/dialogs/NewConnection.cpp \
    src/Utils.cpp \
    src/widgets/ConnectionTab.cpp \
    src/types/Connection.cpp \
    src/dialogs/Password.cpp \
    src/dialogs/About.cpp \
    src/widgets/LongSpinBox.cpp \
    src/widgets/FlatComboBox.cpp \
    src/Query.cpp

HEADERS  += src/MainWindow.hpp \
    src/dialogs/NewConnection.hpp \
    src/Utils.hpp \
    src/widgets/ConnectionTab.hpp \
    src/types/Connection.hpp \
    src/dialogs/Password.hpp \
    src/dialogs/About.hpp \
    src/widgets/LongSpinBox.hpp \
    src/widgets/FlatComboBox.hpp \
    src/Query.hpp

FORMS    += ui/forms/MainWindow.ui \
    ui/dialogs/NewConnection.ui \
    ui/widgets/ConnectionTab.ui \
    ui/dialogs/Password.ui \
    ui/dialogs/About.ui
