#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QDockWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>





namespace widgets {

class DockWrapper : public QDockWidget {

public :

    explicit  DockWrapper( QWidget* parent = nullptr , Qt::WindowFlags flags = Qt::WindowFlags() )  : QDockWidget( parent , flags  ) {

        setStyleSheet( "QDockWidget::title {background: rgb(100,100,120);padding-left: 0px;padding-top: 0px;padding-bottom: 0px;}\nQDockWidget {color: lightgray;font-size: 10pt;}" );

    }
};


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{



    // Ui
    ui->setupUi(this);
    setDockOptions(QMainWindow::ForceTabbedDocks | QMainWindow::AnimatedDocks );
    _settings = new PythonSettings();


    _pos_status = new LineColPosWidget;
    _pos_status->setMaximumHeight(28);
    ui->statusbar->addPermanentWidget( new QWidget , 3 );
    ui->statusbar->addPermanentWidget( _pos_status , 1 );
    connect( ui->documents , &DocumentsViewer::position_changed , _pos_status , &LineColPosWidget::set_position );

    ui->documents->set_class_snippet( ui->actionClassSnippet );
    ui->documents->set_member_function_snippet( ui->actionClassFunctionSnippet );
    ui->documents->set_property_snippet( ui->actionPropertySnippet );
    ui->documents->set_function_snippet( ui->actionFunctionSnippet );
    ui->documents->set_case_snippet( ui->actionCaseSnippet );
    ui->documents->set_loop_list_snippet( ui->actionLoopSequenceSnippet );
    ui->documents->set_loop_dict_snippet( ui->actionLoopMapSnippet);


    // == Dockers ==================================


    QList<QAction*> dockers_actions;

    // == ProjectTreeManager dockers ===============
    {
        _project_tree = new ProjectTreeManager;
        //_project_tree->add_project( QDir(QDir::currentPath()).filePath("app_data/projects"));
        //_project_tree->add_project( "/home/userd/3rdparty/eclipse/tmp/glpvm" );
        //_project_tree->add_project("/home/userd/3rdparty/python/prolog/logicpy-master");

        _dock_project_tree = new DockWrapper;
        _dock_project_tree->setWindowTitle( _project_tree->windowTitle());
        _dock_project_tree->setWidget( _project_tree );
        addDockWidget( Qt::LeftDockWidgetArea , _dock_project_tree );
        dockers_actions.append( _dock_project_tree->toggleViewAction());

        _project_tree->set_new_dir( ui->actionNew_directory );
        _project_tree->set_new_file( ui->actionNewFile );
        _project_tree->set_del_project( ui->actionDelProject );
        _project_tree->set_add_project( ui->actionNewProject );

        connect( _project_tree , &ProjectTreeManager::file_changed , ui->documents , &DocumentsViewer::operator [] );
        connect( _project_tree , &ProjectTreeManager::file_changed , [&]( const QString& ){
            auto *e = ui->documents->current_document();
            if ( e )
                e->setFocus();
        });
        connect( _project_tree , &ProjectTreeManager::new_file_added , ui->documents , &DocumentsViewer::operator [] );
        connect( _project_tree , &ProjectTreeManager::new_dir_added , ui->documents , &DocumentsViewer::operator [] );
        connect( _project_tree , &ProjectTreeManager::file_deleted , this , &MainWindow::__on_file_deleted__);
        connect( _project_tree , &ProjectTreeManager::dir_deleted , this , &MainWindow::__on_dir_deleted__ );
        connect( _project_tree , &ProjectTreeManager::item_renamed, this , &MainWindow::__on_item_renamed__);
        connect( _project_tree , &ProjectTreeManager::project_dir_changed , _settings , &PythonSettings::set_project_dir );
        connect( _project_tree , &ProjectTreeManager::project_dir_changed , [&]( const QString& ){
            __set_enable_project_tools__(true);
        });
        connect(  ui->actionDelProject, &QAction::triggered , [&]( bool ){
           if ( _project_tree->is_empty() )
               __set_enable_project_tools__(false);
        });


    }


    // == FileStructure dockers ===============
    {
        _py_structure= new FileStructure;
        _dock_py_structure = new DockWrapper;
        _dock_py_structure ->setWidget( _py_structure );
        _dock_py_structure->setWindowTitle( _py_structure->windowTitle());
        addDockWidget( Qt::LeftDockWidgetArea , _dock_py_structure );
        dockers_actions.append( _dock_py_structure->toggleViewAction());


        connect( _py_structure , &FileStructure::position_selected , [&](int line ,int col ){
            auto* editor = ui->documents->current_document();
            if ( editor )
                editor->scroll_to(line, col);
        } );
        connect( _py_structure->refresh() , &QAction::triggered, [&]( bool ) {
            auto* editor = ui->documents->current_document();
            if ( editor ){
                _py_structure->update( editor->id() , editor->filename().absoluteFilePath() );
            }

        });
        connect( _dock_py_structure, &DockWrapper::visibilityChanged , [&]( bool value ) {
            if ( value )
                _py_structure->refresh()->trigger();
        });
        connect( _project_tree , &ProjectTreeManager::file_changed , [&](){
            _py_structure->refresh()->trigger();
        });


    }



    // == FramesWidget dockers ===============
    {
        _frames_stack = new FramesWidget;
        _dock_frames_stack = new DockWrapper;
        _dock_frames_stack->setWindowTitle( _frames_stack->windowTitle());
        _dock_frames_stack->setWidget( _frames_stack );
        addDockWidget( Qt::RightDockWidgetArea , _dock_frames_stack );
        dockers_actions.append( _dock_frames_stack->toggleViewAction());

        connect( _frames_stack, &FramesWidget::frame_selected , [&]( size_t /*pid*/ , const QString& filename , int line ){
            auto * edit = ui->documents->operator [](filename );
            if ( edit )
                edit->scroll_to( line );

        });

    }


    // == DebuggerConsole dockers ===============
    {
        _debugger_console = new DebuggerConsole;
        _dock_debugger_console = new DockWrapper;
        _dock_debugger_console->setWindowTitle( _debugger_console->windowTitle());
        _dock_debugger_console->setWidget( _debugger_console );
        addDockWidget( Qt::BottomDockWidgetArea , _dock_debugger_console );
        dockers_actions.append( _dock_debugger_console->toggleViewAction());

        connect( _debugger_console, &DebuggerConsole::framestack , _frames_stack , &FramesWidget::set_framestack );
        connect( _debugger_console , &DebuggerConsole::break_at_line , [&]( const QString& filename , int line ){

            QFileInfo info(filename);
            if ( !info.exists() )
                return;


            te::TextEditor* editor = ui->documents->operator [](filename);
            if ( editor ){
                editor->set_debug_line( line );
                _debugger_console->set_cmdline_focus();
            }
        });
        connect( _debugger_console, &DebuggerConsole::debugging_is_done , [&](){

            ui->documents->for_each_document( [&]( te::TextEditor* e ){

                e->set_debugging_mode(false);

                // files outside project dir are readonly ( for now )
                if ( !_project_tree->is_relative( e->filename().absoluteFilePath() ) )
                    e->setReadOnly(true);

                e->update();
            });


        });
        connect( ui->documents, &DocumentsViewer::breaks_changed, _debugger_console , &DebuggerConsole::breaks_changed );
        connect( _frames_stack, &FramesWidget::frame_selected , _debugger_console, &DebuggerConsole::frame_selected );

    }

    // == VarsWidget dockers ===============
    {
        _vars = new VarsWidget;
        _dock_vars = new DockWrapper;
        _dock_vars->setWindowTitle( _vars->windowTitle());
        _dock_vars->setWidget( _vars );
        addDockWidget( Qt::RightDockWidgetArea , _dock_vars );
        dockers_actions.append( _dock_vars->toggleViewAction());

        connect( _vars, &VarsWidget::var_added , _debugger_console , &DebuggerConsole::add_var );
        connect( _vars, &VarsWidget::var_deleted , _debugger_console , &DebuggerConsole::del_var );
        connect( _vars, &VarsWidget::reload_vars , _debugger_console , &DebuggerConsole::reload_vars );
        connect( _debugger_console , &DebuggerConsole::set_var , _vars, &VarsWidget::set_var );
        connect( _debugger_console , &DebuggerConsole::set_vars , _vars, &VarsWidget::set_vars );
    }

    // == SearchWidget dockers ===============
    {
        _search = new SearchWidget;

        _dock_search = new DockWrapper;
        _dock_search->setWindowTitle( _search->windowTitle());
        _dock_search->setWidget( _search );
        addDockWidget( Qt::BottomDockWidgetArea , _dock_search );
        dockers_actions.append( _dock_search->toggleViewAction());

        connect( _project_tree, &ProjectTreeManager::project_dir_changed, [&](const QString& path ){
            _search->set_search_dir( path );
        });

        connect( ui->documents , &DocumentsViewer::document_changed , [&]( text_editor::TextEditor* e ){
            if ( e ) {
                _search->set_current_file( e->filename() );
            }
        });
        connect( _search , &SearchWidget::file_selected , [&]( const QString& name , int line ){
            QFileInfo file( name );
            if ( file.exists() ){
                auto* editor = ui->documents->operator [](file.absoluteFilePath());
                editor->scroll_to(line);
            }

        });
        connect( _search , &SearchWidget::file_changed , [&]( const QString& path ) {

            auto* editor = ui->documents->operator []( path );
            if ( editor )
                editor->reload_content();
        });


    }

    // == End Dockers ==================================




    // == Finalize dockers ===============
    tabifyDockWidget(_dock_frames_stack,_dock_vars);
    tabifyDockWidget(_dock_search,_dock_debugger_console);
    ui->menu_dockers->addActions( dockers_actions );
    _dock_search->hide();
    _dock_debugger_console->hide();


    // == Finalize toolbars ===============
    ui->toolBar->insertSeparator( ui->actionRun  );
    ui->toolBar->insertSeparator( ui->actionNewFile  );


    //ui->toolBar->insertAction( sep, _project_tree->new_dir() );
    //ui->toolBar->insertAction( _project_tree->new_dir() , ui->actionOpenFile );
    //ui->toolBar->insertAction( ui->actionOpenFile , _project_tree->new_file() );


    // == Prepare deubgger python implementation ===============
    try {

        if ( !_dbg_files.is_init() )
            _dbg_files.init();

        qDebug() << "Debugger dir = " << _dbg_files.debugger_directory();
        qDebug() << "main =" << _dbg_files.main_name();
        qDebug() << "bdb = " << _dbg_files.bdb_name();
        qDebug() << "cmd = " << _dbg_files.cmd_name();
        qDebug() << "dbg = " << _dbg_files.dbg_name();


    }catch ( const std::exception& e ){
        qDebug() << QString::fromStdString( e.what()  );
        exit(0);
    }


    // == Initial state
    _settings->set_debugger_dir( _dbg_files.debugger_directory() );
    _py_structure->set_script_path( _dbg_files.py_structure_script() );

    __set_enable_project_tools__( false );


    load_settings();
}


MainWindow::~MainWindow()
{

}

void MainWindow::load_settings() {

    QString config_dir = QStandardPaths::writableLocation ( QStandardPaths::AppConfigLocation );
    QSettings settings( QDir(config_dir).absoluteFilePath("config.ini") , QSettings::IniFormat );


    _settings->load_settings(settings);
    _project_tree->load_settings( settings );
}

void MainWindow::save_settings() {

    QString config_dir = QStandardPaths::writableLocation ( QStandardPaths::AppConfigLocation );
    QSettings settings( QDir(config_dir).absoluteFilePath("config.ini") , QSettings::IniFormat );

    _settings->save_settings( settings );
    _project_tree->save_settings( settings );
}

void MainWindow::closeEvent(QCloseEvent* ) {

    ui->documents->save_and_close_all();
    _debugger_console->stop_all_threads();


    save_settings();

}

void MainWindow::__set_enable_project_tools__( bool value ) {

    ui->actionDelProject->setEnabled(value);
    ui->actionNewFile->setEnabled(value);
    ui->actionOpenFile->setEnabled(value);
    ui->actionSave->setEnabled(value);
    ui->actionNew_directory->setEnabled(value);


    _project_tree->set_enable_project_tools( value );
}


void MainWindow::__on_item_renamed__( const QString& old_path , const QString& new_path) {

    auto* e = ui->documents->find(old_path);
    if ( e ) {
        e->set_filename( QFileInfo(new_path));
    }

}

void MainWindow::__on_file_deleted__(const QString & path ){

    auto* e = ui->documents->find(path);
    if ( e )
        // the file's just deleted. so close it without save it
        e->exit(false);

}

void MainWindow::__on_dir_deleted__(const QString & path ){


    for ( int pos = 0; pos < ui->documents->num_documents(); ){
        auto* e = ui->documents->at(pos);
        if ( e->filename().absoluteFilePath().startsWith( path ) ){
            // the file belongs to a directory that's just deleted. so close it without save it
            e->exit(false);
        }else ++pos;
    }

    if ( _project_tree->is_empty() ){
        ui->actionNewFile->setEnabled(false);
        ui->actionNew_directory->setEnabled(false);
        ui->actionOpenFile->setEnabled(false);
    }

}


dbg::QBreakInfoList MainWindow::get_breakpoints() {

    dbg::QBreakInfoList breaks;
    for( int pos = 0 ; pos < ui->documents->num_documents(); ++pos ){
        auto* editor = ui->documents->at(pos);

        for ( auto& b : editor->get_breakpoints() )
            breaks.append( b );

    }

    return breaks;

}


void MainWindow::on_actionRun_triggered()
{
    if ( _settings->save_before_run() )
        ui->documents->save_all();


    auto* editor =  ui->documents->current_document();
    if ( !editor )
        return;


    _dock_debugger_console->show();

    qDebug() << "run "<< editor->filename().absoluteFilePath();

    if ( !_settings->is_interpreter_ok() ) {
        QMessageBox::warning( this, "Python interpreter settings"  , "A python interpreter is not setted yet.\nUse the 'Settings' toolbar button to set it.");
        return;
    }

    _debugger_console->set_interpreter( _settings->cmd() , _settings->path() , _settings->environ() , _dbg_files.debugger_directory()   );
    _debugger_console->run_script( editor->filename().absoluteFilePath(), _project_tree->cur_project() , _settings->args()  , get_breakpoints() );
}


void MainWindow::on_actionDebug_triggered()
{
    if ( _settings->save_before_run() ){
        ui->documents->save_all();
    }


    auto* editor =  ui->documents->current_document();
    if ( !editor )
        return;



    ui->documents->for_each_document( []( te::TextEditor* e ){
        e->set_debugging_mode(true);
    });

    _dock_debugger_console->show();



    qDebug() << "debug "<< editor->filename().absoluteFilePath();

    if ( !_settings->is_interpreter_ok() ) {
        QMessageBox::warning( this, "Python interpreter settings"  , "A python interpreter is not setted yet.\nUse the 'Settings' toolbar button to set it.");
        return;
    }


    _debugger_console->set_interpreter( _settings->cmd() , _settings->path() , _settings->environ(), _dbg_files.debugger_directory()  );
    _debugger_console->debug_script( editor->filename().absoluteFilePath(), _project_tree->cur_project() , _settings->args(), get_breakpoints() );
}


void MainWindow::on_actionShow_whitespaces_triggered()
{
    auto* editor = ui->documents->current_document();
    if ( !editor )
        return;

    QTextOption opts = editor->document()->defaultTextOption();
    if ( opts.flags() & QTextOption::ShowTabsAndSpaces ) {
        opts.setFlags( opts.flags() & ~QTextOption::ShowTabsAndSpaces );
    }else{
        opts.setFlags( opts.flags() | QTextOption::ShowTabsAndSpaces );
    }
    if ( opts.flags() & QTextOption::ShowLineAndParagraphSeparators ) {
        opts.setFlags( opts.flags() & ~QTextOption::ShowLineAndParagraphSeparators );
    }else{
        opts.setFlags( opts.flags() | QTextOption::ShowLineAndParagraphSeparators );
    }
    if ( opts.flags() & QTextOption::ShowDocumentTerminator ) {
        opts.setFlags( opts.flags() & ~QTextOption::ShowDocumentTerminator );
    }else{
        opts.setFlags( opts.flags() | QTextOption::ShowDocumentTerminator );
    }

    editor->document()->setDefaultTextOption( opts );
}


void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this,"About Qt");
}


void MainWindow::on_actionSettings_triggered()
{
    _settings->show();
}


void MainWindow::on_actionOpenFile_triggered()
{
    QString path = QFileDialog::getOpenFileName(this,"Open a file", QDir().absolutePath(),"*.py" );
    if ( !path.isEmpty() ){
        auto* e = ui->documents->operator [](path);
        if ( e ){
            e->setReadOnly( !_project_tree->is_relative( path ));
        }
    }

}


void MainWindow::on_actionNewProject_triggered()
{
    QString path = QFileDialog::getExistingDirectory(this,"Load a project directory" );
    if ( !path.isEmpty() ){

        if ( !_project_tree->add_project(path) ){
            QMessageBox::critical(this,"Error loading project directory","Cannot read/write files in the directory.\nMake sure you have permission to do that.");
        }else
            __set_enable_project_tools__( true );
    }
}


void MainWindow::on_actionSave_triggered()
{
    auto * e = ui->documents->current_document();
    if ( e ) {
        e->save();
    }
}



void MainWindow::on_actionTabSize1_triggered()
{
    auto* e = ui->documents->current_document();
    if ( e )
        e->set_tab_size( 1 );
}


void MainWindow::on_actionTabSize2_triggered()
{
    auto* e = ui->documents->current_document();
    if ( e )
        e->set_tab_size( 2 );
}


void MainWindow::on_actionTabSize4_triggered()
{
    auto* e = ui->documents->current_document();
    if ( e )
        e->set_tab_size( 4 );
}


void MainWindow::on_actionTabSize8_triggered()
{
    auto* e = ui->documents->current_document();
    if ( e )
        e->set_tab_size( 8 );
}


void MainWindow::on_actionTabSize16_triggered()
{
    auto* e = ui->documents->current_document();
    if ( e )
        e->set_tab_size( 16 );
}





} // widgets



