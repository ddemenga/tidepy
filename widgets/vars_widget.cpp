#include "vars_widget.h"
#include "vars_delegate.h"
#include "ui_vars_widget.h"



namespace widgets {



int VarsWidget::_ids = 0;




VarsWidget::VarsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VarsWidget)
{
    ui->setupUi(this);

    ui->add->setDefaultAction( ui->actionAdd_Watch );
    ui->del->setDefaultAction( ui->actionRemove_Watch );
    ui->reload->setDefaultAction( ui->actionReload );

    _model = new QStandardItemModel();
    _model->setHorizontalHeaderLabels({"name","value","type"});

    ui->table->setModel( _model );
    ui->table->horizontalHeader()->setStretchLastSection(true);
    ui->table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    _delegate = new VarsDelegate;
    ui->table->setItemDelegate( _delegate );
    connect( _delegate ,&VarsDelegate::del_var , [&] ( int row ){

        emit var_deleted( _model->item(row)->data(Qt::UserRole).toInt() );
        _model->removeRow( row );

    });
    connect( _delegate, &VarsDelegate::name_var_changed , this , &VarsWidget::var_name_changed );


}

VarsWidget::~VarsWidget()
{
    delete ui;
}


void VarsWidget::on_actionAdd_Watch_triggered()
{
    int id = _ids++;
    int row = _model->rowCount();

    _model->appendRow( { new QStandardItem("name") , new QStandardItem("") , new QStandardItem("<none>") } );

    QStandardItem* name = _model->item( row );
    name->setData( QVariant( id ), Qt::UserRole  );

    debugger::VarInfo info;
    info.name = "name";
    info.id = id;

    emit var_added( info );
}

void VarsWidget::set_var( const debugger::VarInfo&  info  ) {


    for ( int irow = 0; irow < _model->rowCount(); irow++){

        QStandardItem* name = _model->item(irow);
        int id = name->data(Qt::UserRole).toInt();

        if ( id == info.id ){
            QStandardItem* value = _model->item(irow,1);
            value->setText(info.value);

            QStandardItem* type = _model->item(irow,2);
            type->setText(info.type);

            break;
        }
    }
}

void VarsWidget::set_vars( const debugger::QVarInfoList& vars ) {


    for ( auto& var : vars ){
        set_var( var );
    }
}

void VarsWidget::var_name_changed( int id ) {

    foreach_row( [&]( int irow , int row_id ) {

        if ( row_id == id ){

            QStandardItem* name = _model->item(irow,0);
            QStandardItem* value = _model->item(irow,1);
            QStandardItem* type = _model->item(irow,2);

            debugger::VarInfo info;
            info.id = id;
            info.name = name->text();
            info.value = value->text();
            info.type = type->text();

            emit var_added( info );
        }
    });
}

void VarsWidget::on_actionRemove_Watch_triggered()
{
    QModelIndex index = ui->table->currentIndex();

    if ( index.isValid() ){
        emit var_deleted( index.data(Qt::UserRole).toInt() );
        _model->removeRow( index.row() );
    }

}



void VarsWidget::on_actionReload_triggered()
{
    emit reload_vars();
}



}// widgets





