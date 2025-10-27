TEMPLATE = app

QT       += core gui
QT += core gui widgets network svg
win32: LIBS += -lwinhttp


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += static \
          c++17

RC_ICONS = Images/icon.ico

TARGET = InvictusHOTASConfigurator

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
WIN_DESKTOP

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH +="widgets" \
    "widgets/adv-settings" \
    "widgets/axes" \
    "widgets/axes-curves" \
    "widgets/buttons" \
    "widgets/encoders" \
    "widgets/led" \
    "widgets/pins" \
    "widgets/shift-reg"\


SOURCES += \
    configmanager.cpp \
    mainwindow_style.cpp \
    widgets/axes-curves/axescurvesbutton.cpp \
    widgets/axes-curves/axescurvesprofiles.cpp \
    widgets/board_presets.cpp \
    widgets/centered_cbox.cpp \
    configtofile.cpp \
    converter.cpp \
    deviceconfig.cpp \
    firmwareupdater.cpp \
    hiddevice.cpp \
    main.cpp \
    mainwindow.cpp \
    mousewheelguard.cpp \
    reportconverter.cpp \
    stm_main.c \
    widgets/developer.cpp \
    widgets/device_info.cpp \
    widgets/infolabel.cpp \
    widgets/pins/pinscontroller.cpp \
    widgets/selectfolder.cpp \
    widgets/switchbutton.cpp \
    widgets/adv-settings/advancedsettings.cpp \
    widgets/axes/axes.cpp \
    widgets/axes/axesconfig.cpp \
    widgets/axes-curves/axescurves.cpp \
    widgets/axes-curves/axescurvesconfig.cpp \
    widgets/axes-curves/axescurvesplot.cpp \
    widgets/axes/axesextended.cpp \
    widgets/axes/axestobuttonsslider.cpp \
    widgets/buttons/buttonconfig.cpp \
    widgets/buttons/buttonlogical.cpp \
    widgets/buttons/buttonphysical.cpp \
    widgets/pins/currentconfig.cpp \
    widgets/debugwindow.cpp \
    widgets/encoders/encoders.cpp \
    widgets/encoders/encodersconfig.cpp \
    widgets/adv-settings/flasher.cpp \
    widgets/led/led.cpp \
    widgets/led/ledconfig.cpp \
    widgets/pins/pincombobox.cpp \
    widgets/pins/pinconfig.cpp \
    widgets/shift-reg/shiftregisters.cpp \
    widgets/shift-reg/shiftregistersconfig.cpp

HEADERS += \
    common_defines.h \
    common_types.h \
    configmanager.h \
    widgets/axes-curves/axescurvesbutton.h \
    widgets/axes-curves/axescurvesprofiles.h \
    widgets/board_presets.h \
    widgets/centered_cbox.h \
    configtofile.h \
    converter.h \
    deviceconfig.h \
    firmwareupdater.h \
    global.h \
    hidapi.h \
    hiddevice.h \
    mainwindow.h \
    mousewheelguard.h \
    reportconverter.h \
    stm_main.h \
    widgets/developer.h \
    widgets/device_info.h \
    widgets/force_anchors.h \
    widgets/infolabel.h \
    widgets/pins/pinscontroller.h \
    widgets/selectfolder.h \
    widgets/switchbutton.h \
    version.h \
    widgets/adv-settings/advancedsettings.h \
    widgets/axes/axes.h \
    widgets/axes/axesconfig.h \
    widgets/axes-curves/axescurves.h \
    widgets/axes-curves/axescurvesconfig.h \
    widgets/axes-curves/axescurvesplot.h \
    widgets/axes/axesextended.h \
    widgets/axes/axestobuttonsslider.h \
    widgets/buttons/buttonconfig.h \
    widgets/buttons/buttonlogical.h \
    widgets/buttons/buttonphysical.h \
    widgets/pins/currentconfig.h \
    widgets/debugwindow.h \
    widgets/encoders/encoders.h \
    widgets/encoders/encodersconfig.h \
    widgets/adv-settings/flasher.h \
    widgets/led/led.h \
    widgets/led/ledconfig.h \
    widgets/pins/pincombobox.h \
    widgets/pins/pinconfig.h \
    widgets/shift-reg/shiftregisters.h \
    widgets/shift-reg/shiftregistersconfig.h

FORMS += \
    mainwindow.ui \
    widgets/adv-settings/advancedsettings.ui \
    widgets/axes-curves/axescurvesprofiles.ui \
    widgets/axes/axes.ui \
    widgets/axes/axesconfig.ui \
    widgets/axes-curves/axescurves.ui \
    widgets/axes-curves/axescurvesconfig.ui \
    widgets/axes/axesextended.ui \
    widgets/axes/axestobuttonsslider.ui \
    widgets/buttons/buttonconfig.ui \
    widgets/buttons/buttonlogical.ui \
    widgets/buttons/buttonphysical.ui \
    widgets/developer.ui \
    widgets/pins/currentconfig.ui \
    widgets/debugwindow.ui \
    widgets/encoders/encoders.ui \
    widgets/encoders/encodersconfig.ui \
    widgets/adv-settings/flasher.ui \
    widgets/led/led.ui \
    widgets/led/ledconfig.ui \
    widgets/pins/pincombobox.ui \
    widgets/pins/pinconfig.ui \
    widgets/pins/pinscontroller.ui \
    widgets/selectfolder.ui \
    widgets/shift-reg/shiftregisters.ui \
    widgets/shift-reg/shiftregistersconfig.ui

TRANSLATIONS += \
    Russian.ts \
    Chinese.ts \
    German.ts


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

linux {
    QMAKE_LFLAGS += -no-pie
    LIBS += -ludev
    SOURCES += \
        linux/hidapi.c
}

win32 {
    RC_FILE = winapp.rc
    LIBS += -lhid -lsetupapi
    SOURCES += \
        windows/hidapi.c
}

macx {
    LIBS += -framework CoreFoundation -framework IOkit
    SOURCES += \
    mac/hidapi.c
}
