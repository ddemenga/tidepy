#include <QDebug>

#include "debugger_controller.h"






namespace debugger {



DebuggerController::DebuggerController(QObject *parent)
    : QObject{parent}, _worker(nullptr) , _thread(new QThread )
{

    connect( _thread , &QThread::started , this , &DebuggerController::started );
    connect( _thread , &QThread::finished , this , &DebuggerController::finished );


    _thread->start();


}


DebuggerController::~DebuggerController(){

    if ( _worker )
        set_worker( nullptr );

    _thread->quit();
    _thread->wait();
    delete _thread;
    _thread = nullptr;


}

void DebuggerController::exit(){


}

void DebuggerController::set( const QDir& debugger_dir ) {

    qDebug() << "set debugger contoller";
    if ( _worker == nullptr ) {

        set_worker( new DebuggerProxy );
        _worker->set_debugger_dir( debugger_dir );

        // input
        connect( this , &DebuggerController::start , _worker , &DebuggerProxy::run );
        connect( this , &DebuggerController::send_dbg , _worker , &DebuggerProxy::print_dbg );
        connect( this , &DebuggerController::send_user , _worker , &DebuggerProxy::print_user );

        // output
        connect( _worker, &DebuggerProxy::started , this , &DebuggerController::debugger_started );
        connect( _worker, &DebuggerProxy::finished , this , &DebuggerController::stop_debugger );
        connect( _worker, &DebuggerProxy::finished , this , &DebuggerController::debugger_finished );

        connect( _worker, &DebuggerProxy::failed_to_start , this , &DebuggerController::debugger_failed_to_start );
        connect( _worker, &DebuggerProxy::output , this , &DebuggerController::debugger_output );
        connect( _worker, &DebuggerProxy::output_dbg , this , &DebuggerController::debugger_output_dbg );
        connect( _worker, &DebuggerProxy::error , this , &DebuggerController::debugger_error );
        connect( _worker, &DebuggerProxy::input , this , &DebuggerController::debugger_input );
        connect( this , &DebuggerController::terminate , _worker , &DebuggerProxy::exit );
    }

}

void DebuggerController::stop_debugger(){

    qDebug() << "exit";
    set_worker( nullptr );

}



void DebuggerController::set_worker ( DebuggerProxy* new_worker ) {


    if ( _worker ){

        _worker->setParent(nullptr);
        delete _worker;
        _worker = nullptr;
    }

    if ( new_worker ){

        assert( new_worker->parent() == nullptr ) ;
        _worker = new_worker;
        _worker->moveToThread( _thread );
        qDebug() << "worker=" << _worker << ": parent" << _worker->parent() << ": thread=" << _worker->thread();

    }

}

} // debugger
