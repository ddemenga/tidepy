#ifndef FRAMESDELEGATE_H
#define FRAMESDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>




class FramesDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    explicit  FramesDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

signals:

};

#endif // FRAMESDELEGATE_H
