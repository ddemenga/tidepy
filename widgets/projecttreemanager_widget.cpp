#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>


#include "widgets/projecttreemanager_widget.h"
#include "ui_projecttreemanager_widget.h"


#include <unistd.h>
#include <sys/stat.h>

#include <fstream>
#include <iostream>



namespace widgets {



ProjectTreeManager::ProjectTreeManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectTreeManager)
{
    ui->setupUi(this);



    ui->del_item->setDefaultAction( ui->actionDelete);
    ui->toggle->setDefaultAction( ui->actionToggle );




    connect( ui->project , &ProjectTree::item_renamed , this , &ProjectTreeManager::item_renamed );
    connect( ui->project , &ProjectTree::file_changed , this , &ProjectTreeManager::file_changed );
    connect( ui->project , &ProjectTree::dir_changed , this , &ProjectTreeManager::dir_changed );
    connect( ui->project , &ProjectTree::project_dir_changed , this , &ProjectTreeManager::project_dir_changed );

    connect( ui->actionToggle , &QAction::triggered , ui->project , &ProjectTree::toggle_current );

}

ProjectTreeManager::~ProjectTreeManager()
{
    delete ui;
}


void ProjectTreeManager::load_settings(QSettings& settings) {

    int n = settings.value("projects/count",0).toInt();
    for ( int i =0; i < n; i++ ){
        add_project( settings.value(QString("projects/project%1").arg(i) ).toString() );
    }
}

void ProjectTreeManager::save_settings(QSettings& settings) {

    QList< QModelIndex > projects = ui->project->top_levels();

    settings.setValue("projects/count", projects.size() );
    int i=0;
    for ( auto p : projects ){
        settings.setValue(QString("projects/project%1").arg(i++), p.data(Qt::UserRole).toString() );
    }
}


bool ProjectTreeManager::is_empty() const {

    return ui->project->is_empty();
}

void ProjectTreeManager::set_new_file(QAction* value )  {
    connect( value , &QAction::triggered , [&]( bool ) {
       __on_actionNewFile_triggered__();
    });
    ui->new_file->setDefaultAction(value);
}

void ProjectTreeManager::set_new_dir(QAction* value)  {
    connect( value , &QAction::triggered , [&]( bool ) {
       __on_actionNewDir_triggered__();
    });
     ui->new_dir->setDefaultAction( value );
}

void ProjectTreeManager::set_add_project( QAction* value ){
    ui->add_project->setDefaultAction( value );
}

void ProjectTreeManager::set_del_project( QAction* value ){
    connect( value , &QAction::triggered , [&]( bool ) {
        ui->project->del_cur_project();
    });
    ui->del_project->setDefaultAction( value );
}

QString ProjectTreeManager::cur_project() const {
    return ui->project->cur_dir_project();
}

QStringList ProjectTreeManager::cur_projects() const {
    return ui->project->cur_projects();
}

bool ProjectTreeManager::is_relative( const QString& path ) const {
    for ( auto& project_dir : ui->project->cur_projects() ){
        if ( path.startsWith( project_dir ))
            return true;

    }
    return false;
}


QString ProjectTreeManager::cur_dir() const {
    return ui->project->cur_dir();
}


bool ProjectTreeManager::add_project( const QString& path ) {
    return ui->project->add_project( path );
}

void ProjectTreeManager::__on_actionNewFile_triggered__()
{
    QString dir = ui->project->cur_dir();


    QString path = QFileDialog::getSaveFileName(this,"Enter a filename",dir,"*.py");

    if ( !path.isEmpty() ){

        path  = normalize_path(path , Type::SCRIPT );
        QFile file( path );


        if ( !file.exists() ){

            if ( file.open(QIODevice::WriteOnly) ){

                ui->project->add_path(path);
                {
                    QTextStream txt(&file);
                    txt << "#!/usr/bin/python3\n";
                    txt.flush();
                }
                emit new_file_added(path);

            }else
                QMessageBox::critical(this,"Error", QString("Cannot save file %1").arg(path) );

        }else
            emit file_changed(path);


    }


}


void ProjectTreeManager::__on_actionNewDir_triggered__()
{
    QString dir = ui->project->cur_dir();

    QString path = QFileDialog::getExistingDirectory(this,"Select a directory", dir  );
    if ( !path.isEmpty()) {

        ui->project->add_path( path );

    }


}





void ProjectTreeManager::on_actionDelete_triggered()
{

    QFileInfo info( ui->project->current_path() );

    if ( ui->project->is_project_dir() ){
        QMessageBox::information(this,"Operation not permitted","Use the 'Unload Porject' button");
        return;
    }

    if ( info.exists() ){

        if  ( info.isDir() ){

            QString msg = QString("This will delete permanently the directory: %1 !\nProcede?").arg(info.absoluteFilePath());
            if( QMessageBox::Yes == QMessageBox::critical(this, "About to delete directory", msg, QMessageBox::Yes | QMessageBox::No ,QMessageBox::No )){


                //info.absoluteDir().removeRecursively();

                QDir(info.absoluteFilePath()).removeRecursively();
                ui->project->del_path();
                emit dir_deleted( info.absoluteFilePath() );

            }

        } else {

            if( QMessageBox::Yes == QMessageBox::critical(this, "About to delete file", QString("This will delete permanently the file: %1 !\nProcede?").arg(info.absoluteFilePath()) ,QMessageBox::Yes | QMessageBox::No ,QMessageBox::No)){

                info.absoluteDir().remove(info.absoluteFilePath());


                ui->project->del_path();
                emit file_deleted( info.absoluteFilePath() );

            }
        }
    }

}


void ProjectTreeManager::set_enable_project_tools( bool value ) {

    ui->actionDelete->setEnabled(value);
    ui->actionToggle->setEnabled(value);
}


QString ProjectTreeManager::normalize_path( const QString& path , Type type ){

    if ( ( type == MODULE || type == SCRIPT ) && !path.endsWith(".py") ) {
        return QString("%1.py").arg(path);
    }

    return path;

}



} //widgets
