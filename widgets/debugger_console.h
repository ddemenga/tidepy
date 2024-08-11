#ifndef DEBUGGER_CONSOLE_H
#define DEBUGGER_CONSOLE_H



#include <iostream>
#include <QWidget>
#include <QMap>
#include <QStringList>
#include <QDir>



#include "py_editor_tools/debugger_json_reader.h"
#include "py_editor_tools/debugger_objects.h"
#include "py_editor_tools/debugger_controller.h"
namespace dbg  = debugger;



namespace Ui {
class DebuggerConsole;
}

namespace debugger {
class PythonDebugger;
class PythonThread;
}




namespace widgets {


class DebuggerConsole : public QWidget
{
    Q_OBJECT



public:



    explicit DebuggerConsole(QWidget *parent = nullptr);
    ~DebuggerConsole();

    void set_cmdline_focus();
    QList<QAction*> actions() const ;

public slots:

    void set_interpreter(const QString& cmd , const QStringList& path, const QMap<QString,QString>& env , const QString &dbg_dir);
    void run_script( const QString& , const QString &wdir, const QStringList& args ={},  const debugger::QBreakInfoList &breaks = {});
    void debug_script(  const QString& filepath, const QString& wdir,const QStringList& args ={} , const debugger::QBreakInfoList &breaks ={});
    void _on_output_( const QString& msg );
    void _on_output_dbg_( const QString& msg );
    void _on_display_exp_( const QString& value );
    void _on_error_( const QString& err );
    void _on_input_(bool debug_user);
    void stop_debugger();
    void stop_all_threads();


    void add_var( const dbg::VarInfo& );
    void del_var( int id );
    void reload_vars();

    void breaks_changed( const QString& path , const dbg::QBreakInfoList& );
    void frame_selected(size_t pid , const QString& filename , int line , int iframe );

signals:

    void start_debugger( const QString& cmd, const QString& args  );

    void launch_script( const QString& filepath, const QStringList& args, const QStringList& breaks);
    void set_var( const dbg::VarInfo&  );
    void set_vars( const dbg::QVarInfoList&  );

    void break_at_line( const QString& filename , int line );
    void framestack( const dbg::QFrameInfoList& );
    void debugging_is_done();

    void send_to_debugger(  const QString& );
    void send_to_user(  const QString& );


private slots:
    void __on_started__();
    void __on_finished__();

    void set_cmdline_readonly(bool);



private:

    void start_python_thread(const QString &args);

    bool python_waits_for_cmds();
    void try_to_send_cmd(const QString &cmd, bool to_dbg = true);
    void try_read_vars(int iframe=-1 );
    void try_send_breaks();

    QString get_debugger_args(const QString &run_type, const QString& filepath , const QString &wdir, const QStringList& args_ = {}, const debugger::QBreakInfoList &breaks_ ={} );
    QString get_debugger_breaks( );

    void insert_debug_text(const QString&);
    void insert_error_text(const QString&);

private:

    Ui::DebuggerConsole *ui;

    QString _python,_python_dbg;
    QStringList _python_path;
    QMap<QString,QString> _python_env;

    bool _ok_input = false;
    dbg::DebuggerProxy::InputType _is_dbg_input = dbg::DebuggerProxy::InputType::DBG;

    bool _is_running = false;
    QStringList _rc_messages;

    dbg::DebuggerController* _debugger;

    dbg::Debugger_JsonReader _json_objects;

    QMap< int , dbg::VarInfo > _vars;
    bool _udpdate_vars = false;

    QMap< QString , dbg::QBreakInfoList > _breaks;
    bool _breaks_changed;

};



} // widgets


#endif // DEBUGGER_CONSOLE_H
