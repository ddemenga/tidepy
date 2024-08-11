#include <QMouseEvent>
#include <QDebug>
#include <QToolTip>
#include <QMenu>

#include "decorated_editor_base.h"


namespace text_editor {


DecoratedEditorBase::DecoratedEditorBase( QWidget *parent ) : TextEditBase{parent}
{

}

DecoratedEditorBase::~DecoratedEditorBase() {

}

bool DecoratedEditorBase::event(QEvent *e)
{

    /**if (e->type() == QEvent::ToolTip) {
        QHelpEvent *he = static_cast<QHelpEvent *>(e);

        int offset = lineNumberAreaWidth();
        QTextCursor tc = cursorForPosition({he->x()-offset , he->y()} );
        tc.select(QTextCursor::WordUnderCursor);

        QToolTip::showText( he->globalPos(), QString("The tooltip: ").append(tc.selectedText()) );

        return true;
    }**/
    return TextEditBase::event(e);
}


void DecoratedEditorBase::mousePressEvent(QMouseEvent* e) {
    TextEditBase::mousePressEvent(e);
}

void DecoratedEditorBase::mouseMoveEvent(QMouseEvent* e) {
    TextEditBase::mouseMoveEvent(e);

    QToolTip::hideText();
    //qDebug() <<  "move pos = " << e->pos();
}


void DecoratedEditorBase::mouseReleaseEvent( QMouseEvent* e ) {
    TextEditBase::mouseReleaseEvent(e);
}

void DecoratedEditorBase::mouseDoubleClickEvent(QMouseEvent *e){
    TextEditBase::mouseDoubleClickEvent(e);

    /**QMenu* m = createStandardContextMenu();

    if ( m )
        m->popup( this->mapToGlobal( e->pos() ) );

    e->accept();
    **/
}


void DecoratedEditorBase::contextMenuEvent(QContextMenuEvent *e) {

    QMenu* menu = createStandardContextMenu();

    if ( menu ){

        QTextCursor tc = textCursor();
        if ( tc.hasSelection() ){
            menu->addSeparator();
            for ( QAction* s : _snippets.keys() ){
                QAction* a = menu->addAction( s->text() );
                connect( a, &QAction::triggered , s, &QAction::triggered );
            }
        }

        menu->popup( this->mapToGlobal( e->pos() ) );
    }

    e->accept();

}


/**void DecoratedEditorBase::add_snippet( QAction* a) {
    if ( !_snippets_actions.contains(a) )
        _snippets_actions.append(a);
}

void DecoratedEditorBase::del_snippet( QAction* a ) {
    _snippets_actions.removeOne(a);

}

void DecoratedEditorBase::clear_snipptes() {
    _snippets_actions.clear();
}
**/


} // text_editor
