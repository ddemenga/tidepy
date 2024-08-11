#include <QProcess>
#include <QTextStream>
#include <QFileInfo>

#include "pythoninfo.h"


namespace utils {


QString PythonInfo::get_version( const QString& interpreter ){

    QProcess p;
    p.start(interpreter,{"--version"}, QIODevice::ReadOnly );
    p.waitForFinished();
    QTextStream txt(&p);
    return txt.readAll().toLower();
}


QStringList PythonInfo::get_path(const QString &interpreter)
{
    QProcess p;
    p.start(interpreter,{"-c", R"(import sys;[ print(x) for x in sys.path])"}, QIODevice::ReadOnly );
    p.waitForFinished();
    QTextStream txt(&p);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    return txt.readAll().split('\n',QString::SkipEmptyParts);
#else
    return txt.readAll().split('\n',Qt::SkipEmptyParts);
#endif

}


QStringList PythonInfo::get_environ(const QString &interpreter)
{

    QProcess p;
    p.start(interpreter,{"-c", R"(import os;[ print(f"{x}={v}") for x,v in os.environ.items()])"}, QIODevice::ReadOnly );
    p.waitForFinished();
    QTextStream txt(&p);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    return txt.readAll().split('\n',QString::SkipEmptyParts);
#else
    return txt.readAll().split('\n',Qt::SkipEmptyParts);
#endif


}


} // utils
