#include <QTextBlock>
#include <QTextCursor>


#include "doc_action_shifttab.h"


namespace text_editor {


void  DocAction::set_tab_size( int value ) {

    _tab_size = qMax( 0, value );
    _tab_spaces = _tab_size == 0 ? QString() : QString( _tab_size , QChar(' '));

}



/** == DocShiftTabAction ===================================================== **/

DocShiftTabAction::DocShiftTabAction(QObject *parent) : QObject{parent} {


}


bool DocShiftTabAction::can_handle_keyseq ( QPlainTextEdit* editor ,const QKeySequence& kseq ) {
    auto tc = editor->textCursor();
    return !editor->isReadOnly() && tc.hasSelection() && !tc.hasComplexSelection() && ( _tab == kseq || _shift_tab == kseq );

}


void DocShiftTabAction::next_block(  QTextCursor& tc) const {
    tc.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor );
}
void DocShiftTabAction::sel_next_block(  QTextCursor& tc) const {
    tc.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor );
}

bool DocShiftTabAction::run( QPlainTextEdit* editor , const QKeySequence &kseq) {

    if ( !editor )
        return false;

    auto tc = editor->textCursor();


    int pos_start = tc.selectionStart();
    int pos_end  = tc.selectionEnd();
    int last_block = tc.document()->findBlock( pos_end ).blockNumber();

    //qDebug() << tc.blockNumber() << ":" << tc.positionInBlock() << ":" << tc.selectionStart() << ":" << tc.selectionEnd() ;

    tc.beginEditBlock();

    tc.setPosition( pos_start);

    if ( kseq == _tab ) {
        tc.movePosition( QTextCursor::StartOfBlock );

        for ( ; tc.blockNumber() <= last_block; next_block(tc) ){
            if ( tab_size() == 0 )
               tc.insertText( "\t");
            else
               tc.insertText( tab_spaces() );


            if ( tc.blockNumber() == last_block )
                break;
        }

    }else{

        tc.movePosition( QTextCursor::StartOfBlock );

        for ( ; tc.blockNumber() <= last_block; next_block(tc) ){
            if ( tab_size() == 0 )
               tc.deleteChar();
            else{
               QString text = tc.block().text();

               for ( int i=0, iend = text.size(), tab_size_ = tab_size(); i < iend && text[i].isSpace() &&  i < tab_size_  ; i++){

                   tc.deleteChar();
               }
            }

            if ( tc.blockNumber() == last_block )
                break;
        }

    }

    tc.setPosition( pos_start );
    tc.movePosition( QTextCursor::StartOfBlock );

    for ( ; tc.blockNumber() <= last_block; sel_next_block(tc) ){
        if ( tc.blockNumber() == last_block )
            break;
    }



    tc.endEditBlock();



    return true;

}


/** == DocTabAction ================================================= **/

DocTabAction::DocTabAction(QObject *parent ) :QObject( parent)  {

}



bool DocTabAction::run( QPlainTextEdit* editor , const QKeySequence&  ) {


    QTextCursor tc = editor->textCursor();
    tc.beginEditBlock();
    int pos = tc.positionInBlock();
    tc.insertText( QString(  tab_size() - pos %  tab_size(), QChar(' ') ) );
    editor->setTextCursor( tc );
    tc.endEditBlock();

    return true;
}

bool DocTabAction::can_handle_keyseq (QPlainTextEdit * editor, const QKeySequence& kseq ) {
    static const QKeySequence tab( Qt::Key_Tab );
    auto tc = editor->textCursor();
    return !editor->isReadOnly() && !tc.hasSelection() && kseq == tab;
}

/** == DocReturnAction ================================================= **/


DocReturnAction::DocReturnAction(QObject *parent ) :QObject( parent) {

}

bool DocReturnAction::can_handle_keyseq (QPlainTextEdit * editor, const QKeySequence& kseq ) {
    static const QKeySequence ret( Qt::Key_Return );
    return !editor->isReadOnly() && ret == kseq;
}

bool DocReturnAction::run( QPlainTextEdit*  editor , const QKeySequence&  )  {

    QTextCursor tc = editor->textCursor();
    tc.beginEditBlock();

    int pos = tc.positionInBlock();
    if ( pos == tc.block().length()-1 ) {

        QString text = tc.block().text();

        int i =0;
        for ( auto& c : text  ) {
            if ( c.isSpace() )
                i++;
            else
                break;
        }


        if ( text.size() && text.back() == ':' )
            i+= tab_size();

        tc.insertText("\n", QTextCharFormat());
        if ( i ) {
            tc.insertText( QString( i , ' ')  ) ;
            editor->setTextCursor( tc );
        }

    }else{

        int i = tc.positionInBlock();

        tc.insertText( QString("\n%1").arg(QString( i , ' ')) ) ;
        editor->setTextCursor( tc );

    }
    tc.endEditBlock();

    return true;

}




/** == DocReturnAction ================================================= **/


DocBackspaceAction::DocBackspaceAction(QObject *parent ) :QObject( parent) {

}

bool DocBackspaceAction::can_handle_keyseq (QPlainTextEdit  * editor , const QKeySequence& kseq ) {
    static const QKeySequence ret( Qt::Key_Backspace );
    return !editor->isReadOnly() && ret == kseq && !editor->textCursor().hasSelection() ;
}



bool DocBackspaceAction::run( QPlainTextEdit*  editor , const QKeySequence&  )  {

    QTextCursor tc = editor->textCursor();

    QString text = tc.block().text();


    bool empty = true;
    for ( int pos = tc.positionInBlock()-1 ; pos >= 0 ; --pos ) {

        if ( pos < text.size() )
            empty &= text.at(pos).isSpace();

        if ( !empty)
            break;
    }

    tc.beginEditBlock();
    if ( !empty ){

        tc.deletePreviousChar();
        editor->setTextCursor( tc );

    }else{

        int n = tc.positionInBlock();
        if ( n == 0 ){
            tc.deletePreviousChar();
            editor->setTextCursor( tc );

        }else {

            n %= tab_size();
            for ( int i= n ? n : tab_size() ; i > 0; --i ){
                tc.deletePreviousChar();
            }
        }

        editor->setTextCursor( tc );
    }


    tc.endEditBlock();



    return true;

}

} // text_editor
