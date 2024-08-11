#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QTextBlock>
#include <QFontMetrics>
#include <QFont>
#include <QPainter>
#include <QRect>
#include <QTextEdit>
#include <QShortcut>
#include <QFileDialog>
#include <QRegExp>
#include <QStringListModel>
#include <QFileSystemModel>
#include <QMenu>
#include <QCompleter>
#include <QStringListModel>
#include <QToolTip>
#include <QApplication>
#include <QGuiApplication>
#include <QMessageBox>
#include <QHelpEvent>


#include <cmath>
#include <iostream>
#include <fstream>

#include "editor.h"


#include "filters/filters.h"






namespace text_editor {

/** == QBreakpoint ==================================================================================================== **/
QBreakpoint::QBreakpoint() : _state(ENABLED) {

}

QBreakpoint::~QBreakpoint() {


}




/** == LineNumbersArea ==================================================================================================== **/






LineNumbersArea::LineNumbersArea(TextEditor *parent)
    : QWidget( parent ),_editor(parent)
{
    _bg_color = QColor(200,200,200);
    _bold = QColor(0,0,0);
    _normal = QColor(128,128,128);
    _break_icon.addFile(QString::fromUtf8(":/app_data/icons/gtk-media-record.svg"), QSize(), QIcon::Normal, QIcon::Off);

}

void LineNumbersArea::set_bg_color( const QColor& c){
    _bg_color = c;
}

const QColor LineNumbersArea::bg_color(){
    return _bg_color;
}


unsigned LineNumbersArea::getWidth(){
    return _editor->lineNumberAreaWidth();
}

void LineNumbersArea::updateWidth(){

    int width_ = getWidth();
    if ( width() != width_ ){
        setFixedWidth(width_);
        _editor->setViewportMargins(width_, 0, 0, 0);
    }
}


void LineNumbersArea::updateContents(const QRect &rect, int dy){

    if ( dy )
        scroll(0, dy);
    else
        update(0, rect.y(), width(), rect.height());

    if ( rect.contains( _editor->viewport()->rect() ) ){

        int fontSize = _editor->currentCharFormat().font().pointSize();
        _font.setPointSize(fontSize);
        _font.setStyle( QFont::StyleNormal);
        updateWidth();
    }

}


void LineNumbersArea::paintEvent( QPaintEvent* e) {


    QPainter p(this);
    p.fillRect( e->rect(), _bg_color );

    int current_line = _editor->textCursor().blockNumber() ;
    /*Iterate over all visible text blocks in the document. */
    for( QTextBlock block = _editor->firstVisibleBlock(); block.isVisible(); block = block.next()){

        //# Check if the position of the block is outside of the visible area.
        int blockNumber = block.blockNumber();
        qreal block_top = _editor->blockBoundingGeometry(block).translated(_editor->contentOffset()).top();
        if ( blockNumber == -1 || block_top >= e->rect().bottom() )
            break;


        //# We want the line number for the selected line to be bold.
        if ( blockNumber == current_line ){
            _font.setBold(true);
            p.setPen( _bold);

        } else {

            _font.setBold(false);
            p.setPen(_normal);
        }

        p.setFont( _font );

        //# Draw the line number right justified at the position of the line.
        QRect paint_rect = QRect(0, block_top, width(), _editor->fontMetrics().height());
        p.drawText(paint_rect, Qt::AlignRight, QString::number(blockNumber+1));

        if ( block.userData() ){
            QBreakpoint* bp = dynamic_cast< QBreakpoint* >( block.userData() );
            if ( bp && !bp->is_none() ){

                QRect brect(0, block_top, _editor->fontMetrics().height(), _editor->fontMetrics().height());
                if ( bp->is_enabled() )
                    p.drawPixmap( brect, _break_icon.pixmap( QSize(_editor->fontMetrics().height(),_editor->fontMetrics().height()), QIcon::Mode::Normal, QIcon::State::On ));
                else
                    p.drawPixmap( brect, _break_icon.pixmap( QSize(_editor->fontMetrics().height(),_editor->fontMetrics().height()), QIcon::Mode::Disabled, QIcon::State::On ));
            }
        }

    }


    p.end();

    QWidget::paintEvent(e);
}



void LineNumbersArea::mouseReleaseEvent(QMouseEvent * e ){


    //qDebug() << "y=" <<   e->y() << " offset = " << _editor->contentOffset();

    int h = _editor->fontMetrics().height() , y = e->y() ;
    for( QTextBlock block = _editor->firstVisibleBlock(); block.isVisible() ; block = block.next()){

        int lineno = block.blockNumber();
        qreal block_top = _editor->blockBoundingGeometry(block).translated(_editor->contentOffset()).top();
        if ( block_top <= y && y <= block_top + h ){

            if( block.userData() ){
                QBreakpoint* bp = dynamic_cast< QBreakpoint* >( block.userData() );
                bp->toggle();
            }else{
                block.setUserData( new QBreakpoint() );
            }

            emit line_selected(lineno);
        }

        if ( lineno+1 >= _editor->num_lines() )
            break;
    }
}


/** == QScoreEditor ==================================================================================================== **/



TextEditor::TextEditor(QWidget *parent )
    : DecoratedEditorBase(parent) ,_line_numbers_space(108),_ln(-1)
{
    setUndoRedoEnabled(true);

    setCenterOnScroll(true);

    auto* completer = new QCompleter(this);
    completer->setModel( modelFromFile( completer, ":/app_data/resources/wordlist.txt"));
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    setCompleter(completer);


    QFont mono = QFont("Monospace");
    mono.setFixedPitch(true);
    mono.setStyleHint(QFont::Monospace);
    setFont(mono);


    setTabStopDistance( _tab_size * fontMetrics().horizontalAdvance(' '));
    setLineWrapMode( QPlainTextEdit::NoWrap );


    _line_numbers_area = new LineNumbersArea( this );
    connect( _line_numbers_area , &LineNumbersArea::line_selected , this , &TextEditor::__on__linenumber_selected__ );

    connect( this, &TextEditor::blockCountChanged,  this, &TextEditor::updateLineNumberAreaWidth );
    connect( this ,&TextEditor::blockCountChanged , _line_numbers_area, &LineNumbersArea::updateWidth);
    connect( this, &TextEditor::updateRequest, _line_numbers_area, &LineNumbersArea::updateContents);

    updateLineNumberAreaWidth();

    _extra_selector.set_current_line_bgcolor( palette().alternateBase() );
    _extra_selector.set_debug_line_bgcolor( QColor(200, 240, 220) );
}



TextEditor::~TextEditor(){

    clear_all();
}

void TextEditor::clear_all() {

    for ( auto* snippet : _snippets ){
        delete snippet;
    }
    _snippets.clear();

    clear();

}



int TextEditor::num_lines() const {
    return document()->blockCount();
}

int TextEditor::line() const {
    return textCursor().blockNumber();
}

int TextEditor::column() const {
    auto cur = textCursor();
    return cur.position()  - cur.block().position();
}

QString TextEditor::text() const {
    return document()->toPlainText();
}


bool TextEditor::exit(bool check_dirty){

    if ( check_dirty && is_dirty() ){

        if ( QMessageBox::question(this,tr("Save File"),QString(tr( "The file %1 is not saved yet. Save it now ?" ) ).arg(filename().absoluteFilePath())) == QMessageBox::Yes )
           save();

    }

    emit remove_document(this);

    return true;
}



void TextEditor::set_highlighter( const TextEditorHighlighter& teh ) {

    if ( _highlighter.parser )
        delete _highlighter.parser;

    if ( _highlighter.sh )
        delete _highlighter.sh;

    _highlighter = teh;
}





void   TextEditor::updateLineNumberAreaWidth( ){
    setViewportMargins( lineNumberAreaWidth(), 0, 0, 0);

}


void TextEditor::set_debugging_mode( bool value ){

    _extra_selector.set_debug_line(-1);

    setReadOnly(value);

    do_extra_selections();

    _debugging_mode = value;
}


void TextEditor::clear_extra_selections(){

    _extra_selector.clear();
    setExtraSelections({});
}

void TextEditor::do_extra_selections(){


    _extra_selector.highlight(this);

    auto selections = _extra_selector.lines_selections();
    selections.append( _extra_selector.brackets_selections() );

    setExtraSelections( selections );
}


void TextEditor::set_debug_line( unsigned line ) {

    set_debugging_mode( true );

    _extra_selector.set_debug_line( line -1 );

    if ( line > 0 )
        scroll_to( line );

    do_extra_selections();

}

void TextEditor::scroll_to( unsigned line, unsigned column  ){


    setFocus();
    QTextCursor curs = textCursor();
    QTextBlock block = document()->findBlockByLineNumber(line-1);

    curs.setPosition( block.position() + column  );
    setTextCursor(curs);

    ensureCursorVisible();

}



void TextEditor::__on__linenumber_selected__( int   ) {

    emit breaks_changed( filename().absoluteFilePath() , get_breakpoints() );

    update();
}

void TextEditor::set_breakpoint( int /*lineno*/ , bool /*enabled*/ ) {

    update();
}


dbg::QBreakInfoList TextEditor::get_breakpoints() const {

    dbg::QBreakInfoList bs;

    dbg::BreakInfo b;


    for (QTextBlock block = document()->begin(); block != document()->end(); block = block.next() ){
        if ( block.userData() ){
            QBreakpoint* bp = dynamic_cast<QBreakpoint*>( block.userData() );

            if ( !bp->is_none() ) {

                b.filename = filename().absoluteFilePath();
                b.lineno = block.blockNumber() +1;
                b.enabled = bp->is_enabled();
                //qDebug() << "break at " << b.filename << ":" << b.lineno << ":" << b.enabled;
                bs.append(b);
            }
        }


    }

    return bs;
}

void TextEditor::resizeEvent(QResizeEvent *e) {

    QRect cr = contentsRect();

    QRect rec = QRect( cr.left(), cr.top(), _line_numbers_area->getWidth(), cr.height());
    _line_numbers_area->setGeometry(rec);

    QPlainTextEdit::resizeEvent(e);

}



QAbstractItemModel* TextEditor::modelFromFile(QCompleter* completer, const QString& fileName)
{


    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(completer);

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif


    QStringList words;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty())
            words << line.trimmed();
    }

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    return new QStringListModel(words, completer);
}


} // text_editor


