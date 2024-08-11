#include <QDebug>
#include <QMessageBox>
#include <QStandardPaths>
#include "debugger_files.h"





DebuggerFiles::DebuggerFiles(QObject* parent  ) : QObject( parent) , _init(false) {

    Q_INIT_RESOURCE(debugger_files);

    _debugger_dir.setPath( QStandardPaths::writableLocation( QStandardPaths::AppLocalDataLocation ) );


    if ( !QFileInfo(_debugger_dir.absolutePath()).exists() && !QDir().mkdir(_debugger_dir.absolutePath()) ){

        QString msg = QString("Can't create debugger implementation directory:\n\n%1\n\nTry check write permissions on that dir.").arg( _debugger_dir.absolutePath() );

        QMessageBox::critical( nullptr , "Cannot initialize the application",  msg );
        exit(0);
    }
}

bool DebuggerFiles::is_init () const {
    return _init;
}

QString DebuggerFiles::debugger_directory() const {
    return _debugger_dir.absolutePath();
}

QString DebuggerFiles::main_name() const {
    return _debugger_dir.absoluteFilePath("py_debugger_main.py");
}

QString DebuggerFiles::bdb_name() const {
    return _debugger_dir.absoluteFilePath("py_bdb.py");
}

QString DebuggerFiles::cmd_name() const {
    return _debugger_dir.absoluteFilePath("py_cmd.py");
}

QString DebuggerFiles::dbg_name() const {
    return _debugger_dir.absoluteFilePath("py_dbg.py");
}

QString DebuggerFiles::py_structure_script() const {
    return _debugger_dir.absoluteFilePath("extract_structure.py");
}

void DebuggerFiles::init(){


    copy_file( ":/debugger/py_editor_tools/py_debugger/py_debugger_main.py", _debugger_dir.absoluteFilePath("py_debugger_main.py") );
    copy_file( ":/debugger/py_editor_tools/py_debugger/py_bdb.py" , _debugger_dir.absoluteFilePath("py_bdb.py") );
    copy_file( ":/debugger/py_editor_tools/py_debugger/py_cmd.py" , _debugger_dir.absoluteFilePath("py_cmd.py") );
    copy_file( ":/debugger/py_editor_tools/py_debugger/py_dbg.py" , _debugger_dir.absoluteFilePath("py_dbg.py") );
    copy_file( ":/debugger/py_editor_tools/extract_structure.py" , _debugger_dir.absoluteFilePath("extract_structure.py") );


    _init = true;
}

DebuggerFiles::~DebuggerFiles(){

}



void DebuggerFiles::copy_file( const QString& source_name , const QString& target_name ) {


    QFile source = QFile(source_name);
    QFile target(target_name);

    if ( source.open(QIODevice::ReadOnly) && target.open(QIODevice::WriteOnly)){
        QTextStream in(&source);
        QTextStream out(&target);

        out << in.readAll();

    }else {

        throw std::runtime_error( QString("Can't copy debugger file : ").arg(target_name).toStdString() ) ;
    }

}
