#ifndef PYTHONINFO_H
#define PYTHONINFO_H


#include <QStringList>



namespace utils {


class PythonInfo {

public:

    static QString get_version( const QString& interpreter );
    static QStringList get_path(const QString& interpreter ) ;

    static QStringList get_environ(const QString& interpreter ) ;
};



}// utils

#endif // PYTHONINFO_H
