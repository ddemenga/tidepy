#ifndef DEBUGGERPROXY_H
#define DEBUGGERPROXY_H

#include <QObject>
#include <QDir>


#include <boost/process.hpp>
namespace bp = boost::process;
//#include "pstreams.hpp"



namespace debugger {


class DebuggerProxy : public QObject
{
    Q_OBJECT



public:

    enum InputType { DBG, USER };

    explicit DebuggerProxy(QObject *parent = nullptr);
    virtual ~DebuggerProxy();

    void set_debugger_dir( const QDir& );

    bool event(QEvent *event) override;

public slots:

    void run(const QString &cmd, const QString & );
    void print_dbg(const QString& );
    void print_user(const QString& );
    void exit();

signals:

    void started(int pid);
    void finished();
    void failed_to_start();

    void output(const QString& );
    void output_dbg(const QString& );
    void error(const QString& );
    void input(bool);

private:

    void get_output();

private:

    QDir _debugger_dir;

    bp::ipstream _is; //reading pipe-stream
    bp::opstream _os; //writing pipe-stream
    bp::child* _client;
    //redi::pstream _proc;

    bool _is_exit, _is_first;
    InputType _is_dbg_input;

    QString _args;
};





} //debugger





#endif // DEBUGGERPROXY_H
