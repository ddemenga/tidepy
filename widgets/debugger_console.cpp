#include <QDebug>
#include <QTextBlock>


#include "debugger_console.h"
#include "ui_debugger_console.h"
#include "utils/ansi_escape.h"

namespace widgets {

DebuggerConsole::DebuggerConsole(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebuggerConsole)
{
    ui->setupUi(this);

    QFont mono = QFont("Monospace");
    mono.setFixedPitch(true);
    mono.setStyleHint(QFont::Monospace);
    ui->console->setFont(mono);


    // == interpreter of debugger's responses  =================================

    connect( &_json_objects, &dbg::Debugger_JsonReader::no_break_at , [&]( const QString& filename , int lineno ){
         QTextCharFormat red;
         red.setForeground(QColor(180,100,100));
         QTextCursor tc = ui->console->textCursor();
         tc.insertText( QString("BREAK INFO: no breat at %1:[%2]").arg(filename).arg(lineno) , red );
         tc.insertText("\n",QTextCharFormat());

    } );
    connect( &_json_objects, &dbg::Debugger_JsonReader::go_to_line , this, &DebuggerConsole::break_at_line );
    connect( &_json_objects, &dbg::Debugger_JsonReader::exception , [&]( const QStringList& stack ){
        for ( auto & s : stack )
            _on_error_(s);
    } );
    connect( &_json_objects, &dbg::Debugger_JsonReader::framestack , this , &DebuggerConsole::framestack );
    connect( &_json_objects, &dbg::Debugger_JsonReader::display_var ,this , &DebuggerConsole::set_var );
    connect( &_json_objects, &dbg::Debugger_JsonReader::display_vars ,this , &DebuggerConsole::set_vars );
    connect( &_json_objects, &dbg::Debugger_JsonReader::display_expr , this , &DebuggerConsole::_on_display_exp_ );


    // == Command line =================================
    connect( ui->cmd_line , &QLineEdit::editingFinished , [&](){
        using dbg::DebuggerProxy;

        if ( !ui->cmd_line->hasFocus() )
            return;

        if ( _is_dbg_input == dbg::DebuggerProxy::DBG )
            try_to_send_cmd( QString("cmdline %1").arg( ui->cmd_line->text().trimmed() ) ,  true  );
        else
            try_to_send_cmd( ui->cmd_line->text() , false  );
    } );
    _ok_input = false;


    // == Debugger =================================

    _debugger  = new dbg::DebuggerController;

    connect( _debugger, &dbg::DebuggerController::started , [&](){ qDebug() << "controller started"; });
    connect( _debugger, &dbg::DebuggerController::finished , [&](){ qDebug() << "controller finished"; });
    connect( _debugger, &dbg::DebuggerController::debugger_started , this, &DebuggerConsole::__on_started__ );
    connect( _debugger, &dbg::DebuggerController::debugger_finished , this , &DebuggerConsole::__on_finished__);
    connect( _debugger, &dbg::DebuggerController::debugger_failed_to_start , [&](){ qDebug() << "debugger fail to start"; });
    connect( _debugger, &dbg::DebuggerController::debugger_output , this , &DebuggerConsole::_on_output_);
    connect( _debugger, &dbg::DebuggerController::debugger_output_dbg , this , &DebuggerConsole::_on_output_dbg_);
    connect( _debugger, &dbg::DebuggerController::debugger_error , this , &DebuggerConsole::_on_error_);
    connect( _debugger, &dbg::DebuggerController::debugger_input ,  this , &DebuggerConsole::_on_input_ );

    connect( this , &DebuggerConsole::send_to_debugger , _debugger , &dbg::DebuggerController::send_dbg );
    connect( this , &DebuggerConsole::send_to_user , _debugger , &dbg::DebuggerController::send_user );
    connect( this , &DebuggerConsole::start_debugger,  _debugger , &dbg::DebuggerController::start );



    // == Actions =================================

    ui->btn_run->setDefaultAction( ui->actionRun );
    ui->btn_continue->setDefaultAction( ui->actionContinue );
    ui->btn_step->setDefaultAction( ui->actionStep );
    ui->btn_step_in->setDefaultAction( ui->actionStep_into );
    ui->btn_step_out->setDefaultAction( ui->actionStep_Out );
    ui->btn_stop->setDefaultAction( ui->actionStop );
    ui->btn_clear->setDefaultAction( ui->actionClear );

    connect( ui->actionRun , &QAction::triggered , [&] ( bool ){
        using dbg::DebuggerProxy;

        if ( _is_dbg_input == DebuggerProxy::DBG )
            try_to_send_cmd("r");
    });
    connect( ui->actionContinue , &QAction::triggered , [&] ( bool ){
        using dbg::DebuggerProxy;

        if ( _is_dbg_input == DebuggerProxy::DBG )
            try_to_send_cmd("c");
    });
    connect( ui->actionStep , &QAction::triggered , [&] ( bool ){
        using dbg::DebuggerProxy;

        if ( _is_dbg_input == DebuggerProxy::DBG )
            try_to_send_cmd("n");
    });
    connect( ui->actionStep_into , &QAction::triggered , [&] ( bool ){
        using dbg::DebuggerProxy;

        if ( _is_dbg_input == DebuggerProxy::DBG )
            try_to_send_cmd("s");
    });
    connect( ui->actionStep_Out , &QAction::triggered , [&] ( bool ){
        using dbg::DebuggerProxy;

        if ( _is_dbg_input == DebuggerProxy::DBG )
            try_to_send_cmd("o");
    });
    connect( ui->actionStop , &QAction::triggered , this , &DebuggerConsole::stop_debugger );
    connect( ui->actionStop , &QAction::triggered, _debugger, &dbg::DebuggerController::terminate );
    connect( ui->actionClear , &QAction::triggered, [&]( bool ){ ui->console->clear(); });


}



DebuggerConsole::~DebuggerConsole()
{

    delete _debugger;
}


void DebuggerConsole::start_python_thread(   const QString& args ){

    _debugger->set( _python_dbg );
    emit start_debugger( _python , args );

}

void DebuggerConsole::stop_debugger() {

    set_cmdline_readonly(true);
    _is_running = false;
    _debugger->stop_debugger();

    emit debugging_is_done();
}


void DebuggerConsole::set_interpreter( const QString& cmd , const QStringList& path, const QMap<QString,QString>& env , const QString& dbg_dir ) {

    _python  = cmd;
    _python_dbg = dbg_dir;
    _python_path = path;
    _python_env = env;

}

void DebuggerConsole::run_script(   const QString& filepath , const QString & wdir, const QStringList& args, const dbg::QBreakInfoList &breaks ) {

    if ( _is_running )
        return;

    _is_running = false;
    _udpdate_vars = false;
    _breaks_changed = false;
    _rc_messages.clear();

    ui->console->clear();
    start_python_thread(    get_debugger_args("run",filepath,wdir,args,breaks) );


}

void DebuggerConsole::debug_script(  const QString& filepath, const QString & wdir, const QStringList& args, const dbg::QBreakInfoList &breaks) {

    if ( _is_running )
        return;


    _is_running = false;
    _udpdate_vars = true;
    _breaks_changed = false;
    _rc_messages.clear();


    ui->console->clear();
    start_python_thread(   get_debugger_args("debug", filepath,wdir,args,breaks) );
}

void DebuggerConsole::stop_all_threads(){


}

void DebuggerConsole::set_cmdline_focus(){
    ui->cmd_line->setFocus();
}

QList<QAction*> DebuggerConsole::actions() const {
    return { ui->actionContinue , ui->actionStep, ui->actionStep_into , ui->actionStep_Out , ui->actionStop };
}




bool DebuggerConsole::python_waits_for_cmds() {

    return _is_running  && _ok_input;

}


QString DebuggerConsole::get_debugger_breaks( ) {

    json breaks = json::array();
    for ( auto& bs : _breaks.values() ){

        for ( auto& b : bs ){
            json tmp;
            tmp["filename"] = b.filename.toStdString();
            tmp["lineno"] = b.lineno;
            tmp["enabled"] = b.enabled;
            breaks.insert( breaks.end() , tmp );
        }
    }

    std::stringstream ss; ss <<  breaks;
    return QString::fromStdString(ss.str());
}

QString DebuggerConsole::get_debugger_args( const QString& run_type,const QString& filepath , const QString& wdir, const QStringList& args_, const dbg::QBreakInfoList &breaks_ ) {


    _breaks.clear();


    json data;

    data["run_type"] = run_type.toStdString();
    data["python"] = _python.toStdString();
    data["script"] = filepath.toStdString();

    json path = json::array();
    for ( auto& a : _python_path )
        path.insert( path.end() , a.toStdString() );
    data["path"] = path;

    json env;
    for ( auto it = _python_env.begin(); it != _python_env.end(); ++it  )
        env[it.key().toStdString()] = it.value().toStdString();
    data["env"] = env;

    json args = json::array();
    for ( auto& a : args_ )
        args.insert( args.end() , a.toStdString() );
    data["args"] = args;

    json breaks = json::array();
    for ( auto& b : breaks_ ){

        json tmp;
        tmp["filename"] = b.filename.toStdString();
        tmp["lineno"] = b.lineno;
        tmp["enabled"] = b.enabled;
        breaks.insert( breaks.end() , tmp );

        _breaks[b.filename].append( b );
    }
    data["breaks"] = breaks;
    data["wdir"] = wdir.toStdString();


    std::stringstream ss; ss <<  data;
    return QString::fromStdString(ss.str());


}


void DebuggerConsole::insert_debug_text(const QString& txt) {

    ui->console->insertPlainText( txt  );

    QTextCharFormat f;
    f.setForeground(QColor(100,150,100));
    f.setFontWeight(QFont::Bold);
    QTextCursor c = ui->console->textCursor();
    c.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor , txt.size() );
    c.setCharFormat( f );

    {
    QTextCursor c = ui->console->textCursor();
    c.setCharFormat(QTextCharFormat{});
    ui->console->setTextCursor(c);
    }
}

void DebuggerConsole::insert_error_text(const QString& txt) {

    ui->console->insertPlainText( txt  );

    QTextCharFormat f;
    f.setForeground( txt.startsWith("NameError:") ? QColor(200,100,100): QColor(150,100,100));
    f.setFontWeight(QFont::Bold);
    QTextCursor c = ui->console->textCursor();
    c.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor , txt.size() );
    c.setCharFormat( f );

    {
    QTextCursor c = ui->console->textCursor();
    c.setCharFormat(QTextCharFormat{});
    ui->console->setTextCursor(c);
    }
}


void DebuggerConsole::try_to_send_cmd(const QString& msg , bool to_dbg ) {

    if ( _ok_input ){

        _ok_input= false;
        if ( to_dbg ){
            if ( msg.size() )
                emit send_to_debugger( msg );
        } else
            emit send_to_user( msg );

        set_cmdline_readonly( true );

    }
}

void DebuggerConsole::_on_output_( const QString& msg ){

    if ( msg.isEmpty() )
        return;

    AnsiEscape().set_text_edit(ui->console).format_text( msg );
    //ui->console->insertPlainText( msg );

    QTextBlock block = ui->console->document()->findBlockByLineNumber( ui->console->blockCount()-1 );
    QTextCursor curs = ui->console->textCursor();
    curs.setPosition( block.position()  );
    ui->console->setTextCursor( curs );
    ui->console->ensureCursorVisible();
}


void DebuggerConsole::_on_output_dbg_( const QString& msg ){

    if ( msg.trimmed().isEmpty() )
        return;

    if ( msg.startsWith( "line dbg>") || msg.startsWith( "call dbg>") || msg.startsWith( "exc dbg>") ){
        return;
    }

    if ( !_json_objects.read( msg ) ){

        insert_debug_text(msg);
    }

}

void DebuggerConsole::_on_display_exp_( const QString& value ) {



    //QTextBlock block = ui->console->document()->findBlockByLineNumber( ui->console->blockCount()-1 );
    QTextCursor curs = ui->console->textCursor();
    QTextCharFormat cf = curs.charFormat();
    cf.setForeground(Qt::black);

    curs.movePosition( QTextCursor::End  );
    curs.insertText( QString("\n%1\n").arg(value), cf );

    ui->console->setTextCursor( curs );
    ui->console->ensureCursorVisible();
}


void DebuggerConsole::_on_error_( const QString& err ){

    if ( err.trimmed().isEmpty() )
        return;

    if ( !_json_objects.read( err ) )
        insert_error_text(err);
}

void DebuggerConsole::_on_input_( bool debug_user ){

    using dbg::DebuggerProxy;

    _is_dbg_input = debug_user ? DebuggerProxy::DBG : DebuggerProxy::USER;

    _ok_input = true;

    set_cmdline_readonly( false );

    if ( _is_dbg_input == DebuggerProxy::DBG ){

        if ( _udpdate_vars )
            try_read_vars();

        if ( _breaks_changed )
            try_send_breaks();

        for ( auto msg : _rc_messages ){
            emit send_to_debugger(msg);
        }
        _rc_messages.clear();
    }



}


void DebuggerConsole::__on_started__() {

    _is_running = true;
    qDebug() << "== debugger started ==================================";

}

void DebuggerConsole::__on_finished__() {

    _is_running = false;
    set_cmdline_readonly( true );
    qDebug() << "== debugger is done ==================================";

    emit debugging_is_done();

}


void DebuggerConsole::set_cmdline_readonly(bool value ) {

    if ( value ) {

        ui->cmd_line->blockSignals(true);
        ui->cmd_line->setText("");
        ui->cmd_line->setStyleSheet("");
        ui->cmd_line->blockSignals(false);


    } else {


        ui->cmd_line->blockSignals(true);
        ui->cmd_line->setText("");
        if ( _is_dbg_input == dbg::DebuggerProxy::DBG )
            ui->cmd_line->setStyleSheet("background-color: rgb(200, 240, 220);");
        else
            ui->cmd_line->setStyleSheet("background-color: rgb(240, 200, 220);");
        ui->cmd_line->blockSignals(false);

    }

    ui->cmd_line->setReadOnly(value);
}


void DebuggerConsole::add_var( const dbg::VarInfo& var ) {

    _vars[var.id] = var;


    qDebug() << "var " << var.name << "[" <<  var.id << "] added";
    qDebug() << "Vars:";
    for ( auto k : _vars.keys() ){
        qDebug() << _vars[k].id <<" = " << _vars[k].name;
    }

    _vars[var.id] = var;

    _udpdate_vars = true;

    try_read_vars();

}

void DebuggerConsole::del_var( int id ) {


    if ( _vars.contains(id ) ){

        auto var = _vars[id];
        if ( _vars.remove(id) ){
            qDebug() << "var " << var.name << "[" <<  var.id << "] removed";

            qDebug() << "Vars:";
            for ( auto k : _vars.keys() ){
                qDebug() << _vars[k].id <<" = " << _vars[k].name;
            }
        }

        _udpdate_vars = true;

        try_read_vars();
    }

}

void DebuggerConsole::reload_vars( ) {

    _udpdate_vars = true;

    try_read_vars();

}

void DebuggerConsole::try_read_vars(int iframe) {

    if ( python_waits_for_cmds() ) {

        if ( iframe == -1 ) {
            dbg::QVarInfoList list;
            for( auto& var : _vars.values() )
                list.append(var);

            QString json_msg = _json_objects.dump_vars( list );

            qDebug() << "add_watch_batch " <<  json_msg ;
            emit send_to_debugger( QString("add_watch_batch %1").arg(json_msg) );
        }else
            emit send_to_debugger( QString("display_batch_at_frame %1").arg(iframe) );

        _udpdate_vars = false;

    }
}

void DebuggerConsole::try_send_breaks() {

    if ( python_waits_for_cmds() ) {

        emit send_to_debugger( QString("set_breaks %1").arg( get_debugger_breaks(  ) ) );
        _breaks_changed = false;

    }
}


void DebuggerConsole::breaks_changed(const QString& path , const debugger::QBreakInfoList & breaks ) {

    _breaks[path] = breaks;
    _breaks_changed = true;

    try_send_breaks();

}

void DebuggerConsole::frame_selected( size_t /*pid*/ , const QString& /*filename*/ , int /*line*/ , int iframe ) {
    //qDebug() << pid << ":" << filename << ":" << line << ":" <<iframe;
    try_read_vars(iframe);
}




} // widgets


