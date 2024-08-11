#ifndef FRAMESWIDGET_H
#define FRAMESWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QTreeView>

#include "py_editor_tools/debugger_objects.h"



namespace widgets {


class FramesWidget : public QTreeView {

    Q_OBJECT

public:

    explicit FramesWidget(QWidget *parent = nullptr);
    virtual ~FramesWidget();

public slots:

    void set_framestack( const debugger::QFrameInfoList& );

signals:

    void frame_selected( size_t pid , const QString& filename , int line , int index );

private:

    QStandardItem* find_item( int pid ) ;

private:

    QStandardItemModel* _model;

};


} // widgets



#endif // FRAMESWIDGET_H
