#ifndef DEBUGGERFILES_H
#define DEBUGGERFILES_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QTemporaryFile>





class DebuggerFiles : public QObject
{
    Q_OBJECT


public :


    explicit DebuggerFiles(QObject* parent = nullptr );

    bool is_init () const;

    QString debugger_directory() const;

    QString main_name() const ;

    QString bdb_name() const ;

    QString cmd_name() const ;

    QString dbg_name() const ;

    QString py_structure_script() const;

    void init();

    ~DebuggerFiles();

private:

    void copy_file( const QString& source_name , const QString& target_name ) ;


private:


    bool _init  = false ;

    QDir _debugger_dir;
};





#endif // DEBUGGERFILES_H
