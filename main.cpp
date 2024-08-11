#include <QApplication>

#include "mainwindow.h"
#include <QDebug>
#include <QLoggingCategory>




int main(int argc, char* argv[]) {


    QApplication a(argc, argv );


#ifdef QT_NO_DEBUG
    QLoggingCategory::setFilterRules( "*.debug=false");
#endif

    qSetMessagePattern("[%{time hh:mm:ss.z} %{file} %{line} : %{type} : %{threadid} ] - %{message}");
    qRegisterMetaType< debugger::FrameInfo >("debugger::FrameInfo");
    qRegisterMetaType< debugger::QFrameInfoList >("debugger::QFrameInfoList");
    qRegisterMetaType< debugger::VarInfo >("debugger::VarInfo");
    qRegisterMetaType< debugger::QVarInfoList >("debugger::QVarInfoList");
    qRegisterMetaType< debugger::BreakInfo >("debugger::BreakInfo");
    qRegisterMetaType< debugger::QBreakInfoList >("debugger::QBreakInfoList");
    qRegisterMetaType<QTextCursor>("QTextCursor");

    qDebug() << "main thread";


    widgets::MainWindow w;
    w.show();



    a.exec();



    exit(0);

}

