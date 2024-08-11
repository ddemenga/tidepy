#ifndef VARS_DELEGATE_H
#define VARS_DELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>




class VarsDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    explicit  VarsDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

signals:

    void del_var( int row ) const;
    void name_var_changed( int row  ) const;
};


#endif // VARS_DELEGATE_H
