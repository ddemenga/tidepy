#include <QModelIndex>
#include <QHeaderView>

#include "treectrl.h"






namespace widgets {




TreeCtrl::TreeCtrl(QWidget *parent)
    : QTreeView{parent}
{

    _model = new QStandardItemModel;// {"frame 1", "frame 2"} );
    setModel( _model );

    setAlternatingRowColors(true);
    setEditTriggers( QAbstractItemView::SelectedClicked );
}

TreeCtrl::~TreeCtrl () {

}

void TreeCtrl::set_columns(const QStringList& value ) {

    if ( value.size() ){
        _model->setHorizontalHeaderLabels(value);

    }else{
        header()->hide();
    }
}


bool TreeCtrl::is_empty() const {
    return _model->invisibleRootItem()->rowCount() == 0;
}

QStandardItem* TreeCtrl::item( const QModelIndex& index ) const {

    if ( index.isValid() )
        return _model->itemFromIndex( index );
    return nullptr;

}

QModelIndex TreeCtrl::add_item( const QString& text , const QModelIndex& parent ) {

    return add_item( text , QIcon(), parent );

}

QModelIndex TreeCtrl::add_item( const QString& text , const QIcon& icon , const QModelIndex& parent ) {

    auto* iparent = parent.isValid() ? _model->itemFromIndex( parent ) : _model->invisibleRootItem();

    QStandardItem* item =  new QStandardItem(icon, text ) ;

    iparent->appendRow({item});

    return item->index();

}

QList< QModelIndex > TreeCtrl::top_levels() const {

    QList< QModelIndex > indices;
    auto* root = _model->invisibleRootItem();
    for ( int i=0; i < root->rowCount(); ++i ){
        auto * item = root->child(i);
        if ( item ){
            indices.append( item->index() );
        }
    }

    return indices;

}


}//
