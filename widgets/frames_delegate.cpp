#include <QLineEdit>
#include "frames_delegate.h"


FramesDelegate::FramesDelegate( QObject *parent ): QStyledItemDelegate{parent}
{

}



QWidget *FramesDelegate::createEditor(QWidget */*parent*/, const QStyleOptionViewItem &/* option */,  const QModelIndex & /*index*/ ) const
{
    return nullptr;

    /**if ( index.column() == 1 )


    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(true);


    return editor;
    **/
}


void FramesDelegate::setEditorData(QWidget */*editor*/,  const QModelIndex &/*index*/) const
{
    //QString value = index.model()->data(index, Qt::EditRole).toString();

    //QLineEdit *le = static_cast<QLineEdit*>(editor);
    //le->setText(value);
}


void FramesDelegate::setModelData(QWidget */*editor*/, QAbstractItemModel */*model*/, const QModelIndex &/*index*/) const
{
    /**if ( index.column() == 1 )
        return ;

    QLineEdit * le = static_cast<QLineEdit*>(editor);

    model->setData(index, le->text() , Qt::EditRole);
    **/
}


void FramesDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

