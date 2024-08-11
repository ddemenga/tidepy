#include <QDebug>
#include <QString>
#include <QEvent>
#include <QStandardPaths>



#include "debugger_proxy.h"






namespace debugger {


namespace {


void null2space( QString& value ){

    for ( int i=0; i < value.size(); ++i )
        if ( value[i] == QChar(0x00) )
            value[i] = ' ';
}

}







DebuggerProxy::DebuggerProxy(QObject *parent) : QObject{parent} , _client(nullptr)
{

    _is_exit = false;
    _is_dbg_input = DBG;

}

DebuggerProxy::~DebuggerProxy(){

    if ( _client ){
        _client->terminate();
        _client->wait();
        delete _client;
    }
}

void DebuggerProxy::set_debugger_dir( const QDir& value ) {
    _debugger_dir = value;
}

bool DebuggerProxy::event(QEvent *event){


    /**if ( event->type() == QEvent::ThreadChange  ){
        qDebug() << "Thread changed for " << this;
    }**/

    return QObject::event(event);
}

void DebuggerProxy::run( const QString& cmd, const QString & args ){

    assert( !cmd.isEmpty() );

    if ( _client &&  _client->running())
        return;

    qDebug() << "starting client... \n" ;


    auto env = boost::this_process::environment();
    env["_"]= cmd.toStdString();
    env["TERM"] = "xterm-256color";
    env["PYTHONBREAKPOINT"] = "0";
    env["TIDEPY_PATH"] = _debugger_dir.absolutePath().toStdString();

    if ( !_client )
        _client = new bp::child( cmd.toStdString(), bp::args+={ _debugger_dir.absoluteFilePath("py_debugger_main.py").toStdString() },  env , bp::std_out > _is , bp::std_in < _os );

    if ( _client->running() ){
        emit started(getpid());

        _is_first = true;
        _is_exit = false;

        _args = args;

        get_output();

    }else{

        emit failed_to_start();

    }
}

void DebuggerProxy::get_output( ){

    //qDebug() << "get_output " ;
    if ( !_client )
        return;

    std::string line;
    while (_client->running() && std::getline( _is, line)  ){

        QString msg = QString::fromStdString(line);

        if ( !msg.isEmpty()){

            null2space(msg);
            //qDebug() << "rec:" << msg ;

            if ( msg.startsWith("IND:") ){
                //_is_dbg_input = DBG;


                if ( _is_first ){

                    _is_first = false;
                    _os << "IND:" << _args.toStdString() << std::endl;
                    continue;

                } else if ( _is_exit ){
                    qDebug() << "send exit";

                    _os << "IND:EXIT:" << std::endl;
                    exit();
                    emit finished();

                } else
                    emit input(true);
                break;

            } else if ( msg.startsWith("INU:") ){
                //_is_dbg_input = USER;

                emit input(false);
                break;

            }else if ( msg.startsWith("DBG:EXIT" ) ){
                _is_exit = true;
                emit output_dbg( QString("EXIT").append("\n") );

                continue;

            } else if ( msg.startsWith("DBG:") ){
                if ( msg.size()>4)
                    emit output_dbg( msg.mid(4).append("\n") );

            } else if ( msg.startsWith("ERR:") ){
                if ( msg.size()>4)
                    emit error( msg.mid(4).append("\n") );

            }else if ( msg.startsWith("OUT:") ){
                if ( msg.size()>4)
                    emit output( msg.mid(4).append("\n") );

            }else {
                emit output( msg.append("\n") );
            }

        }
    }
}



void DebuggerProxy::print_dbg( const QString& msg ) {


    if ( _client && _client->running() ) { //&& _is_dbg_input == DBG ) {

        QString data = QString("IND:").append(msg);
        //qDebug() << "print proxy "  << data ;
        _os << data.toStdString() << std::endl;
    }

    get_output();

}

void DebuggerProxy::print_user( const QString& msg ) {


    if ( _client && _client->running() ) {//&&  _is_dbg_input == USER ){

        QString data = QString("INU:").append(msg);
        //qDebug() << "print to user"  << data ;
        _os << data.toStdString() << std::endl;

        get_output();
    }

    //_is_dbg_input = DBG;



}


void DebuggerProxy::exit() {


    if (_client && _client->running()){

        qDebug() << "exit the proxy";

        _client->terminate();
        _client->wait();

    }


}



}
