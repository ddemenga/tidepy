#include <cmath>

#include <QCompleter>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QFileDialog>
#include <QShortcut>
#include <QTextBlock>
#include <QAction>



#include "actions/doc_action_shifttab.h"
#include "texteditbase.h"





namespace text_editor {


/**

I know the question is about three years old but.
I just had a similar problem.
If you want to replace the tabs with spaces you could overwrite the keyPressEvent function and create a new event which contains spaces instead of the tab.

 def keyPressEvent (self, oEvent):
    if oEvent.key () == Qt.Key_Tab:
        oEvent = QKeyEvent (QEvent.KeyPress
            , Qt.Key_Space
            , Qt.KeyboardModifiers(oEvent.nativeModifiers())
            , "    ")
    super().keyPressEvent (oEvent)
**/



TextEditBase::TextEditBase(QWidget *parent)
: QPlainTextEdit(parent), _highlighter({nullptr,nullptr}), _tab_size(4), _active(false), _dirty(false)
{

    setMouseTracking(true);


    auto* shift_tab = new DocShiftTabAction;
    _actions[ QKeySequence( Qt::Key_Tab ) ] = shift_tab;
    _actions[ QKeySequence( Qt::SHIFT | Qt::Key_Tab ) ] = shift_tab;
    _actions[ QKeySequence( Qt::Key_Return) ] = new DocReturnAction;
    _actions[ QKeySequence( Qt::Key_Tab) ] =  new DocTabAction;
    _actions[ QKeySequence( Qt::Key_Backspace )] = new DocBackspaceAction;

    connect( this , &QPlainTextEdit::modificationChanged , [&](bool){ emit state_changed(_id); });

    _shortcuts.append( new QShortcut(QKeySequence(tr("Ctrl+Y")),this) );
    QObject::connect( _shortcuts.back(), &QShortcut::activated, [&] (){ redo(); });

    _shortcuts.append( new QShortcut(QKeySequence(tr("Ctrl+S")),this) );
    QObject::connect( _shortcuts.back(), &QShortcut::activated, [&] (){ save();  });

    _shortcuts.append( new QShortcut(QKeySequence(tr("Ctrl+G")),this) );
    QObject::connect( _shortcuts.back(), &QShortcut::activated, [&] (){ save_as(); });

    _shortcuts.append( new QShortcut(QKeySequence(tr("Ctrl+O")),this) );
    QObject::connect( _shortcuts.back(), &QShortcut::activated, [&] (){ open(); });


    connect( this , &QPlainTextEdit::cursorPositionChanged , [&] {
        auto cursor = textCursor();
        emit cursor_pos_changed( cursor.blockNumber() +1, cursor.columnNumber() +1 , cursor.position() );
    });

}

TextEditBase::~TextEditBase()
{
}

unsigned TextEditBase::lineNumberAreaWidth(){

    int digits = std::log10( qMax(1,blockCount()) ) + 3;

    int space=3+fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}


void TextEditBase::set_tab_size( int value ) {
    _tab_size = qMax( 1 , value );

    for ( Snippet* s : _snippets.values() ){
        s->filter->set_tab_size( _tab_size );
    }

    for ( DocAction* a : _actions.values() )
        a->set_tab_size(value);
}


void TextEditBase::add_snippet( QAction* a, text_editor::filters::Filter* s, const QKeySequence& kseq ) {

    if ( !s || !a )
        return;


    _snippets[a] = new Snippet(this, s , kseq );

    connect( &_snippets[a]->kseq, &QShortcut::activated, [a]( ) {  a->triggered(); });

    // save the connection to be able to disconnect it from a when the editor will be destroyed
    _snippets[a]->con = connect( a , &QAction::triggered , [a,this]( bool ){

        if (is_active()){
            if ( _snippets.contains(a) )
                do_filter( *(_snippets[a]->filter) );
        }
    } );

}





void TextEditBase::setCompleter(QCompleter *completer)
{
    if (_completer)
        QObject::disconnect(_completer, 0, this, 0);

    _completer = completer;

    if (!_completer)
        return;

    _completer->setWidget(this);
    _completer->setCompletionMode(QCompleter::PopupCompletion);
    _completer->setCaseSensitivity(Qt::CaseSensitive);
    _completer->setFilterMode(Qt::MatchStartsWith);
    QObject::connect( _completer, QOverload<const QString&>::of( &QCompleter::activated ), this, &TextEditBase::insert_completion  );

}


QCompleter *TextEditBase::completer() const
{
    return _completer;
}

void TextEditBase::insert_completion(const QString& completion)
{
    if (_completer->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - _completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString TextEditBase::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void TextEditBase::mouseReleaseEvent( QMouseEvent* e ) {

    QPlainTextEdit::mouseReleaseEvent(e);

    auto cur = textCursor();

    emit  cursor_pos_changed( cur.blockNumber() +1, cur.columnNumber() +1 , cur.position());
}


void TextEditBase::focusInEvent(QFocusEvent *e)
{
    if (_completer)
        _completer->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}


/**
 * @brief TextEditBase::keyPressEvent   Handle Key Events. Mainly based on Qt examples.
 * @param e
 */
void TextEditBase::keyPressEvent(QKeyEvent *e)
{


    // == From Qt examples ===============
    if (_completer && _completer->popup()->isVisible()) {

        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }




    int k = e->key() + e->modifiers();
    QKeySequence kseq( k  );
    for ( auto it = _actions.begin(); it != _actions.end() ; ++it ){

        auto* action = *it ;
        if ( action->can_handle_keyseq(this,kseq)  ){
             if ( action->run( this , kseq ) ){
                return;

             }else

                break;
        }

    }


    // == From Qt examples ===============
    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if ( _completer && !isShortcut ) {
        QPlainTextEdit::keyPressEvent(e);

    }

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!_completer || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3
                      || eow.contains(e->text().right(1)))) {
        _completer->popup()->hide();
        return;
    }

    if (completionPrefix != _completer->completionPrefix()) {
        _completer->setCompletionPrefix(completionPrefix);
        _completer->popup()->setCurrentIndex(_completer->completionModel()->index(0, 0));

        QRect cr = cursorRect();
        cr.setWidth(_completer->popup()->sizeHintForColumn(0)
                    + _completer->popup()->verticalScrollBar()->sizeHint().width());
        _completer->complete(cr); // popup it up!

    }



}


void TextEditBase::wheelEvent(QWheelEvent* e) {

    if ( e->modifiers()  == Qt::ControlModifier  ){
        QFont f = font();

        if  ( e->angleDelta().y() > 0 )
            f.setPointSize( f.pointSize()+1);
        else
            f.setPointSize( qMax( 1 , f.pointSize()-1 ) );

        setFont(f);

    }else
        QPlainTextEdit::wheelEvent(e);
}



int TextEditBase::id() const {
    return _id;
}

void TextEditBase::set_id(int value ) {
    _id = value;
}



void TextEditBase::set_filename(const QFileInfo& value){
    _filename =  value;
    filename_changed( _id , _filename.fileName() );
}

const QFileInfo TextEditBase::filename() const{
    return _filename;
}

bool TextEditBase::is_active() const {
    return _active;
}

void TextEditBase::set_active( bool value ) {

    //qDebug() << filename() << ":" << value ;
    _active = value;
}


bool TextEditBase::is_dirty() const{
    return document()->isModified();
}


bool TextEditBase::open(){
    auto filename = QFileDialog::getOpenFileName(this,tr("Open score file..."),"./",tr("Score (*.sc)"));
    if ( filename != "" )
        return open( filename );
    return false;

}

bool TextEditBase::open( const QString& filepath ){

    QFile score(filepath);
    if (!score.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;


    _filename = QFileInfo(filepath);

    QTextStream out(&score);
    setPlainText( out.readAll() );

    return true;
}



bool TextEditBase::save_as(){

    //QString path = _filename.isDir() ? _filename.absoluteFilePath() : _filename.absoluteDir().absolutePath();


    QString filter = _filename.absoluteFilePath().endsWith(".py") ? _filename.absoluteFilePath() : _filename.absoluteFilePath()+".py";
    QString filename = QFileDialog::getSaveFileName(this, QString("%1: %2").arg( tr( "Save file ")).arg( _filename.fileName() )
                                                    ,  filter ,tr("Python (*.py)"));
    if ( filename.isEmpty() )
        return false;

    QFileInfo info(filename);
    if ( info.suffix() != "py" )
        filename += ".py";

    document()->setModified( true );
    return save( filename );

}

bool TextEditBase::save( ){

    if ( _filename.isDir() || !_filename.isFile() ) return save_as();

    return save( _filename.absoluteFilePath() );
}


bool TextEditBase::save( const QString& filepath ){

    if ( !is_dirty() )
        return true;

    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    _filename = filepath;

    {
        QTextStream out( &file );
        out << toPlainText();
    }


    emit filename_changed( _id , _filename.fileName() );

    document()->setModified(false);

    return true;
}


void TextEditBase::clear_text(){
    clear();


}

void TextEditBase::load_text( const QString& value ){

    setPlainText( value );


}

void TextEditBase::load_file( const QString& value ){

    QFile src(value);
    if ( src.open( QIODevice::ReadOnly ) ){

        _filename = QFileInfo(value);
        QTextStream text(&src);
        load_text( text.readAll() );
        emit filename_changed( _id , _filename.fileName() );
    }

    document()->setModified(false);

}


void TextEditBase::reload_content() {

    if ( _filename.exists() ){

        clear();

        QFile src( _filename.absoluteFilePath() );
        if ( src.open( QIODevice::ReadOnly ) ){

            QTextStream text(&src);
            load_text( text.readAll() );

        }
    }
}





} //
