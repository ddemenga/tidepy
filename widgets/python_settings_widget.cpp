#include <QStringList>
#include <QFileInfo>
#include <QDebug>
#include <QLineEdit>
#include <QContextMenuEvent>
#include <QMenu>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QSettings>




#include "python_settings_widget.h"
#include "ui_python_settings_widget.h"
#include "utils/pythoninfo.h"






namespace widgets {

PythonSettings::PythonSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PythonSettings)
{
    ui->setupUi(this);


    ui->environ->setColumnCount(2);
    ui->environ->horizontalHeader()->setStretchLastSection(true);




    connect( ui->path->itemDelegate(), &QAbstractItemDelegate::commitData ,[&](QWidget* w)
    {
        auto* item = ui->path->currentItem();

        if ( !w || !item )
            return;

        QLineEdit* editor = reinterpret_cast<QLineEdit*>(w);
        if (!editor)
            return;

        QFileInfo info( editor->text() );
        if ( !info.exists() ){

            item->setText( item->data(Qt::UserRole).toString() );

        }else {
            item->setData( Qt::UserRole , editor->text() );
        }

        _dirty = true;


    });

    _cur_interpreter = -1;
}

PythonSettings::~PythonSettings()
{
    for ( auto* v : _inters.values() )
        delete v;

    delete ui;
}

bool PythonSettings::is_interpreter_ok() {
    return _cur_interpreter != -1;
}

bool PythonSettings::save_before_run() const {
    return ui->save_before->isChecked();
}

void PythonSettings::save_settings(QSettings& settings) {

    settings.setValue( "interpreter/count",_inters.count() );
    settings.setValue( "interpreter/pdir_to_path", ui->pdir_to_path->isChecked() );
    settings.setValue( "interpreter/save_before_run", ui->save_before->isChecked() );
    settings.setValue( "interpreter/args", ui->args->text() );

    for ( int i : _inters.keys() ){
        if ( i == 0 ){
            settings.setValue( "interpreter/label" ,  _inters[ i ]->label);
            settings.setValue( "interpreter/cmd" , _inters[ i ]->cmd );
            settings.setValue( "interpreter/path", _inters[ i ]->path );
        }else{
            settings.setValue( QString("interpreter/label%1").arg(i) ,  _inters[ i ]->label);
            settings.setValue( QString("interpreter/cmd%1").arg(i) , _inters[ i ]->cmd );
            settings.setValue( QString("interpreter/path%1").arg(i), _inters[ i ]->path );
        }
    }

}

void PythonSettings::load_settings(QSettings &settings) {


    if ( settings.contains("interpreter/count") ){

        int n = settings.value("interpreter/count").toInt();
        for ( int i =0; i < n ; i++ ){
            _inters[ i ] = new Interpreter;

            if ( i == 0 ){

                _inters[ i ]->label = settings.value( "interpreter/label").toString();
                _inters[ i ]->cmd = settings.value( "interpreter/cmd").toString();
                _inters[ i ]->path = settings.value("interpreter/path").toStringList();
                _inters[ i ]->environ = utils::PythonInfo::get_environ("/home/userd/projects/ai-torch/bin/python3");

            }else{
                _inters[ i ]->label = settings.value( QString("interpreter/label%1").arg(i)).toString();
                _inters[ i ]->cmd = settings.value( QString("interpreter/cmd%1").arg(i)).toString();
                _inters[ i ]->path = settings.value( QString("interpreter/path%1").arg(i)).toStringList();
                _inters[ i ]->environ = utils::PythonInfo::get_environ("/home/userd/projects/ai-torch/bin/python3");

            }

            _inters[ i ]->environ.append("NUMEXPR_NUM_THREADS=1");
            _inters[ i ]->environ.append("OMP_NUM_THREADS=1");

            ui->interpreter->addItem( QString("%1  [ %2 ]").arg(_inters[ i ]->label).arg( _inters[ i]->cmd ), _inters[ i ]->cmd );

        }

        ui->pdir_to_path->setChecked( settings.value( "interpreter/pdir_to_path" , true ).toBool() );
        ui->save_before->setChecked( settings.value( "interpreter/save_before_run" , true).toBool()  );
        ui->args->setText( settings.value( "interpreter/args" ).toString() );
    }

    ui->interpreter->addItem("add ... ","");

    if ( _inters.size() )
        change_interpreter(0);
}


void PythonSettings::set_debugger_dir( const QString& value ) {

    _debugger_dir = value;

}

const QString& PythonSettings::cmd() const {
    int index = ui->interpreter->currentIndex();
    return _inters[index]->cmd;
}

QStringList PythonSettings::args() const{
    return ui->args->text().trimmed().split(' ');
}

QStringList PythonSettings::path() const {

    QStringList path;
    for ( int i =0;  i< ui->path->count(); i++ ){
        path.append( ui->path->item(i)->text() );
    }

    path.insert(0, _debugger_dir );

    if ( add_project_dir_to_path() && !_cur_project_dir.isEmpty()  && !path.contains( _cur_project_dir ))
        path.insert(0, _cur_project_dir );



    return path;

}

bool PythonSettings::add_project_dir_to_path() const {
    return ui->pdir_to_path->isChecked();
}


QMap<QString,QString> PythonSettings::environ() const {
    int index = ui->interpreter->currentIndex();

    QMap<QString,QString> env;
    for ( QString var : _inters[index]->environ ){

        QStringList tokens = var.split('=');
        env[tokens[0].trimmed()] = tokens[1].trimmed();
    }

    return env;
}


void PythonSettings::set_project_dir( const QString& value ) {
    _cur_project_dir = value;
}

void PythonSettings::on_btn_close_clicked()
{
    close();
}

void PythonSettings::change_interpreter( int index ){


    if ( _inters.contains(index) ) {

        _cur_interpreter  = index;

        _inters[_cur_interpreter]->environ.sort();

        ui->path->clear();
        ui->path->addItems( _inters[index]->path );

        for (int i = 0; i < ui->path->count(); i++) {
            auto* item = ui->path->item(i);
            item->setData(Qt::UserRole, item->text());
            item->setFlags( item->flags() | Qt::ItemIsEditable );
        }

        ui->environ->clearContents();
        int irow = 0;
        for ( QString var : _inters[index]->environ ){

            QStringList tokens = var.split('=');

            if  ( irow++ >= ui->environ->rowCount() )
                ui->environ->insertRow( ui->environ->rowCount() );

            ui->environ->setItem( irow-1, 0 , new QTableWidgetItem( tokens[0] ));
            ui->environ->setItem( irow-1, 1 , new QTableWidgetItem( tokens[1] ));

        }


        while ( ui->environ->rowCount() > _inters[index]->environ.size() ){
            ui->environ->removeRow( ui->environ->rowCount() -1 );
        }

    }
}

void PythonSettings::save_interpreter_state() {


    if ( _cur_interpreter < 0  || _cur_interpreter >= _inters.size() )
        return;


    _inters[_cur_interpreter]->path.clear();
    for ( int irow = 0; irow < ui->path->count(); ++irow )
        _inters[_cur_interpreter]->path.append( ui->path->item(irow)->text() );


    _inters[_cur_interpreter]->environ.clear();
    for ( int irow = 0; irow < ui->environ->rowCount(); ++irow ){
        QString name = ui->environ->item( irow, 0 )->text();
        QString value = ui->environ->item( irow, 1 )->text();
        _inters[_cur_interpreter]->environ.append(QString(name).append("=").append(value));
    }

}

void PythonSettings::contextMenuEvent(QContextMenuEvent *e) {

    QMenu* menu = new QMenu();
    menu->addAction ( ui->actionAdd );
    menu->addAction ( ui->actionRemove );
    if ( ui->settings->currentIndex() == 0 ){
        menu->addAction ( ui->actionUp );
        menu->addAction ( ui->actionDown );
    }
    menu->popup( this->mapToGlobal( e->pos() ) );


    e->accept();

}



void PythonSettings::on_actionAdd_triggered()
{
    if ( ui->settings->currentIndex() == 0 ){
        //qDebug() << "Add path";
        int row = ui->path->currentRow();

        auto* item = new QListWidgetItem("");
        item->setData(Qt::UserRole, item->text());
        item->setFlags( item->flags() | Qt::ItemIsEditable );
        ui->path->insertItem(row, item );

    }else{
        //qDebug() << "Add var";
        int row = ui->environ->currentRow();
        ui->environ->insertRow(row);
        ui->environ->setItem( row, 0 , new QTableWidgetItem( "NAME" ));
        ui->environ->setItem( row, 1 , new QTableWidgetItem( "VALUE" ));
    }

    update();
}


void PythonSettings::on_actionRemove_triggered()
{
    if ( ui->settings->currentIndex() == 0 ){
        //qDebug() << "Remove path" << ui->path->currentItem()->text();
        ui->path->takeItem( ui->path->currentRow());

    }else{

        //qDebug() << "Remove var" << ui->environ->currentRow() << ":" << ui->environ->currentItem()->text();
        ui->environ->removeRow( ui->environ->currentRow() );
    }
    update();

}




void PythonSettings::on_interpreter_activated(int index)
{
    save_interpreter_state();

    if ( index == ui->interpreter->count()-1 ){
        qDebug() << "add interpreter";
        QString path = QFileDialog::getOpenFileName( this , "Select the interpreter program" , QDir::currentPath()  );
        QFileInfo info (path);

        if ( info.exists() && info.isExecutable() ){

            qDebug() << path;
            int i = _inters.size();
            _inters[ i ] = new Interpreter;
            _inters[ i ]->label = utils::PythonInfo::get_version(path).trimmed();
            _inters[ i ]->cmd  = path;
            _inters[ i ]->path = utils::PythonInfo::get_path(path);
            _inters[ i ]->environ = utils::PythonInfo::get_environ(path);

            // try to change the combobox
            ui->interpreter->insertItem( i , QString("%1  [ %2 ]").arg(_inters[ i ]->label).arg( _inters[ i ]->cmd ) );
            ui->interpreter->setCurrentIndex( i );
            update();

            // show path tab
            ui->settings->setCurrentIndex(0);

            change_interpreter(i);
        }
    }else{
        qDebug() << "interpreter " << ui->interpreter->itemText(index);
        change_interpreter(index);
    }
}


void PythonSettings::on_actionUp_triggered()
{
    if ( ui->settings->currentIndex() == 0 ){

        int irow = ui->path->currentRow();

        if ( irow > 0 ){
            auto* item = ui->path->takeItem(irow);
            ui->path->insertItem( irow-1, item );
            ui->path->setCurrentRow(irow-1);
        }
    }
}


void PythonSettings::on_actionDown_triggered()
{
    if ( ui->settings->currentIndex() == 0 ){

        int irow = ui->path->currentRow();

        if ( irow +1 < ui->path->count() ){
            auto* item = ui->path->takeItem(irow);
            ui->path->insertItem( irow+1, item );
            ui->path->setCurrentRow(irow+1);
        }
    }
}

} //widgets
