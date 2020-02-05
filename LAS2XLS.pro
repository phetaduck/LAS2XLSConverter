QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# QXlsx code for Application Qt project
QXLSX_PARENTPATH=./QXlsx/         # current QXlsx path is . (. means curret directory)
QXLSX_HEADERPATH=./QXlsx/header/  # current QXlsx header path is ./header/
QXLSX_SOURCEPATH=./QXlsx/source/  # current QXlsx source path is ./source/
include(./QXlsx/QXlsx.pri)

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ./Sources/las2xlsconverter.cpp \
    ./Sources/las_curve.cpp \
    ./Sources/las_curvenames_parser.cpp \
    ./Sources/las_curvevalues_parser.cpp \
    ./Sources/las_file_parser.cpp \
    ./Sources/las_section_parser.cpp \
    ./Sources/las_well.cpp \
    main.cpp \
    ./Sources/xls_exporter.cpp \
    mosquitto.cpp \
    mqtt_impl.cpp

INCLUDEPATH += ./Headers \
    C:/Dev/Mosquitto/devel

HEADERS += \
    ./Headers/las2xlsconverter.h \
    ./Headers/las_curve.h \
    ./Headers/las_curvenames_parser.h \
    ./Headers/las_curvevalues_parser.h \
    ./Headers/las_file_parser.h \
    ./Headers/las_section_parser.h \
    ./Headers/las_well.h \
    ./Headers/supported_sections.h \
    ./Headers/xls_exporter.h \
    Headers/mosquitto.hpp \
    mqtt_impl.h

LIBS += \
    -LC:/Dev/Mosquitto/devel -lmosquitto

FORMS += \
  las2xlsconverter.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
