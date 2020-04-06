TEMPLATE = app
TARGET = linty

qtHaveModule(printsupport): QT += printsupport xml
requires(qtConfig(fontdialog))

SOURCES += \
    CodeEditor.cpp \
    Highlighter.cpp \
    Icon.cpp \
    LintOptions.cpp \
    Linter.cpp \
    Log.cpp \
    MainWindow.cpp \
    ProgressWindow.cpp \
    Settings.cpp \
    Worker.cpp \
    main.cpp

HEADERS += \
    CodeEditor.h \
    Highlighter.h \
    Icon.h \
    Jenkins.h \
    LintOptions.h \
    Linter.h \
    Log.h \
    MainWindow.h \
    ProgressWindow.h \
    Settings.h \
    Worker.h

FORMS += \
    LintOptions.ui \
    MainWindow.ui \
    ProgressWindow.ui

RESOURCES += \
    linty.qrc \

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/tutorials/notepad
INSTALLS += target

DISTFILES +=

