#include <QLineEdit>
#include "vars_delegate.h"





VarsDelegate::VarsDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}



QWidget *VarsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */,  const QModelIndex & index ) const
{

    if ( index.column() >= 1 )
        return nullptr;

    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(true);


    return editor;
}


void VarsDelegate::setEditorData(QWidget *editor,  const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    QLineEdit *le = static_cast<QLineEdit*>(editor);
    le->setText(value);
}


void VarsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if ( index.column() >= 1 )
        return ;

    QLineEdit * le = static_cast<QLineEdit*>(editor);

    int id = index.data(Qt::UserRole).toInt() ;

    QString old_text = model->data(index).toString();
    if ( old_text != le->text() ){
        auto var_name = le->text().trimmed();
        model->setData(index, var_name  , Qt::EditRole);
        emit name_var_changed( id ) ;
    }


    if ( le->text().isEmpty() )
        emit del_var( id );
}


void VarsDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

