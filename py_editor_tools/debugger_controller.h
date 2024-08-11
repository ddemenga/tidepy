#ifndef DEBUGGER_CONTROLLER_H
#define DEBUGGER_CONTROLLER_H


#include <QThread>
#include <QDir>



#include "py_editor_tools/debugger_proxy.h"




namespace debugger {



class DebuggerController : public QObject
{
    Q_OBJECT

public:

    explicit DebuggerController(QObject *parent = nullptr);
    virtual ~DebuggerController();



public slots:

    void set(const QDir &debugger_dir);
    void stop_debugger();
    void exit();

signals :

    void start( const QString & , const QString &);
    void terminate();

    void started();
    void finished();

    void debugger_started();
    void debugger_finished();
    void debugger_failed_to_start();

    void debugger_output(const QString& );
    void debugger_output_dbg(const QString& );
    void debugger_error(const QString& );
    void debugger_input(bool);

    void send_dbg( const QString& );
    void send_user( const QString& );


private:

    void set_worker ( DebuggerProxy* );


private:

    DebuggerProxy* _worker;

    QThread* _thread;

};




} // debugger



#endif // DEBUGGER_CONTROLLER_H
