QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 console




# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS += `pkg-config --cflags python3`  -I/usr/include/python3.10 -I/usr/include/x86_64-linux-gnu/python3.10


SOURCES += \
        debugger_files.cpp \
        editor/actions/doc_action_shifttab.cpp \
        editor/decorated_editor_base.cpp \
        editor/editor.cpp \
        editor/extraselections.cpp \
        editor/filters/filters.cpp \
        editor/isyntax_highlight.cpp \
        editor/lang_highlighters/hightlighter_python.cpp \
        editor/syntaxhighlighter.cpp \
        editor/texteditbase.cpp \
        main.cpp \
        mainwindow.cpp \
        py_editor_tools/debugger_controller.cpp \
        py_editor_tools/debugger_json_reader.cpp \
        py_editor_tools/debugger_proxy.cpp \
        utils/ansi_escape.cpp \
        utils/pythoninfo.cpp \
        widgets/debugger_console.cpp \
        widgets/dir_selector_widget.cpp \
        widgets/documentsviewer.cpp \
        widgets/filestructure.cpp \
        widgets/frames_delegate.cpp \
        widgets/frames_widget.cpp \
        widgets/line_pos_widgets.cpp \
        widgets/projecttree_widget.cpp \
        widgets/projecttreemanager_widget.cpp \
        widgets/python_settings_widget.cpp \
        widgets/search_result.cpp \
        widgets/search_widget.cpp \
        widgets/treectrl.cpp \
        widgets/vars_delegate.cpp \
        widgets/vars_widget.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



HEADERS += \
    debugger_files.h \
    editor/actions/doc_action_shifttab.h \
    editor/decorated_editor_base.h \
    editor/editor.h \
    editor/extraselections.h \
    editor/filters/filters.h \
    editor/find.hpp \
    editor/grammar_base.hpp \
    editor/grammar_utils.hpp \
    editor/isyntax_highlight.h \
    editor/lang_highlighters/hightlighter_python.h \
    editor/syntaxhighlighter.h \
    editor/texteditbase.h \
    mainwindow.h \
    py_editor_tools/debugger_controller.h \
    py_editor_tools/debugger_json_reader.h \
    py_editor_tools/debugger_objects.h \
    py_editor_tools/debugger_proxy.h \
    utils/ansi_escape.h \
    utils/pythoninfo.h \
    widgets/debugger_console.h \
    widgets/dir_selector_widget.h \
    widgets/documentsviewer.h \
    widgets/filestructure.h \
    widgets/frames_delegate.h \
    widgets/frames_widget.h \
    widgets/line_pos_widgets.h \
    widgets/projecttree_widget.h \
    widgets/projecttreemanager_widget.h \
    widgets/python_settings_widget.h \
    widgets/search_result.h \
    widgets/search_widget.h \
    widgets/treectrl.h \
    widgets/vars_delegate.h \
    widgets/vars_widget.h

DISTFILES += \
    BUGS \
    BUILD \
    CMakeLists.txt \
    HACKING \
    LICENCE.BOOST.md \
    LICENSE \
    LICENSE.MIT.md \
    README.md \
    TODO \
    app_data/resources/python_builtins.txt \
    app_data/resources/wordlist.txt \
    mytools/__init__.py \
    mytools/extract_structure.py \
    hello.txt \
    mytools/py_bdb.py \
    mytools/py_cmd.py \
    mytools/py_dbg.py \
    py_editor_tools/extract_structure.py \
    py_editor_tools/py_debugger/__init__.py \
    py_editor_tools/py_debugger/py_bdb.py \
    py_editor_tools/py_debugger/py_cmd.py \
    py_editor_tools/py_debugger/py_dbg.py \
    py_editor_tools/py_debugger/py_debugger_main.py \
    py_module.py

FORMS += \
    mainwindow.ui \
    widgets/debugger_console.ui \
    widgets/dir_selector_widget.ui \
    widgets/documentsviewer.ui \
    widgets/filestructure.ui \
    widgets/line_pos_widgets.ui \
    widgets/projecttreemanager_widget.ui \
    widgets/python_settings_widget.ui \
    widgets/search_widget.ui \
    widgets/vars_widget.ui

RESOURCES += \
    customcompleter.qrc \
    debugger_files.qrc \
    icons.qrc








