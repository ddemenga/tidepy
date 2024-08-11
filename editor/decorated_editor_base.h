#ifndef DECORATEDEDITORBASE_H
#define DECORATEDEDITORBASE_H


#include <QAction>
#include <QList>

#include "texteditbase.h"



namespace text_editor {



class DecoratedEditorBase : public TextEditBase
{
    Q_OBJECT
public:

    explicit DecoratedEditorBase(QWidget *parent = nullptr);
    virtual ~DecoratedEditorBase();


    /**void add_snippet( QAction* );
    void del_snippet( QAction* );
    void clear_snipptes();
    **/


signals:


protected:

    bool event(QEvent*) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mouseReleaseEvent( QMouseEvent*  ) override;
    void contextMenuEvent(QContextMenuEvent *) override;


protected:

    //QList<QAction*> _snippets_actions;



};



} // text_editor



#endif // DECORATEDEDITORBASE_H
