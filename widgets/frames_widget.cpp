#include <QDebug>
#include <QHeaderView>
#include "frames_widget.h"
#include "frames_delegate.h"


namespace widgets {



FramesWidget::FramesWidget(QWidget *parent)
    : QTreeView{parent}
{
    setWindowTitle("Frames");

    _model = new QStandardItemModel;// {"frame 1", "frame 2"} );
    _model->setHorizontalHeaderLabels({"thread","filename","line","func","index"});

    setModel( _model );
    header()->stretchLastSection();
    header()->setDefaultAlignment(Qt::AlignLeft);
    setAlternatingRowColors(true);
    setItemDelegate( new FramesDelegate );

    connect( this , &QTreeView::clicked , [&](const QModelIndex &index){

        if ( index.isValid() ){

            QStandardItem* item = _model->itemFromIndex(index);
            QStandardItem* p = item->parent();
            if ( !p )
                p = _model->invisibleRootItem();

            QStandardItem* pid = p->child( index.row(), 0);
            QStandardItem* filename = p->child( index.row(), 1);
            QStandardItem* line = p->child( index.row(), 2);
            QStandardItem* findex = p->child( index.row(), 4);

            emit frame_selected( pid->data(Qt::UserRole).value<size_t>() , filename->text() , line->text().toInt(), findex->text().toInt() );

        }
    });

}

FramesWidget::~FramesWidget() {

}

QStandardItem* FramesWidget::find_item( int pid ) {

    for ( int irow = 0; irow < _model->rowCount(); ++irow ){
        QStandardItem * item = _model->item( irow  );
        assert ( item );
        int item_pid = item->data( Qt::UserRole).value<size_t>();
        if ( pid == item_pid )
            return item;
    }

    return _model->invisibleRootItem();
}

void FramesWidget::set_framestack( const debugger::QFrameInfoList& frames ) {


    _model->removeRows(0,_model->rowCount());


    for ( auto& f : frames ){
        QVariant vpid;
        vpid.setValue<size_t>(f.pid);

        QStandardItem *pid = new QStandardItem(QString("Thread [ pid=%1 ] [ id=%2 ]").arg(QString::number(f.pid)).arg(QString::number(f.id)) );
        pid->setData(  vpid , Qt::UserRole );

        QStandardItem *filename = new QStandardItem(f.filename);
        QStandardItem *lineno = new QStandardItem( QString::number(f.lineno) );
        QStandardItem *func = new QStandardItem( f.func );
        QStandardItem *index = new QStandardItem( QString::number(f.index) );

        find_item( f.pid )->appendRow({ pid, filename,lineno,func,index});

    }

    expandAll();

}


} //widgets
