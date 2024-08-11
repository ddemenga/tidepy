#include <QFileDialog>

#include "dir_selector_widget.h"
#include "ui_dir_selector_widget.h"






DirSelector::DirSelector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DirSelector)
{
    ui->setupUi(this);
    ui->btn->setDefaultAction( ui->actionChange_Dir );


}

DirSelector::~DirSelector()
{
    delete ui;
}

void DirSelector::set_directory( const QString& value ) {

    if ( QFileInfo(value).exists())
        ui->text->setText( value );
    else
        ui->text->setText( QDir::currentPath() );
}

QString DirSelector::get_directory() {
    return ui->text->text();
}

void DirSelector::on_actionChange_Dir_triggered()
{
    QString path = QFileDialog::getExistingDirectory( this , ui->actionChange_Dir->toolTip() , QDir::currentPath()  );
    if ( !path.isEmpty() ){

        emit dir_changed( path );

    }
}

