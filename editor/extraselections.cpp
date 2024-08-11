#include <QTextCursor>
#include <QTextBlock>
#include <QColor>
#include <QDebug>

#include "extraselections.h"


namespace text_editor {





ExtraSelections::ExtraSelections(QObject *parent)
    : QObject{parent}
{
    _current_line_bgcolor = QColor(180,180,180);
}

const QList<QTextEdit::ExtraSelection>& ExtraSelections::lines_selections() const {
    return _lines;
}

const QList<QTextEdit::ExtraSelection>& ExtraSelections::brackets_selections() const {
    return _brackets;
}


void ExtraSelections::clear() {
    _lines.clear();
    _brackets.clear();
}

void ExtraSelections::set_debug_line( unsigned line ) {
    _debug_line = line;
}



void ExtraSelections::set_current_line_bgcolor(const QBrush& value ){
    _current_line_bgcolor = value;
}

void ExtraSelections::set_debug_line_bgcolor(const QBrush & value){
    _dbg_line_bgcolor = value;
}

void ExtraSelections::highlight( QPlainTextEdit* editor ) {

    clear();

    _lines.push_back(  QTextEdit::ExtraSelection() );
    auto& sel = _lines.back();
    sel.format.setProperty( QTextFormat::FullWidthSelection, true);
    sel.format.setBackground( _current_line_bgcolor );
    sel.cursor = editor->textCursor();


    QTextBlock block = editor->document()->findBlockByLineNumber( _debug_line );
    if ( block.isValid() && block.isVisible() ){

        _lines.push_back(  QTextEdit::ExtraSelection() );
        auto& sel = _lines.back();
        sel.format.setProperty( QTextFormat::FullWidthSelection, true);
        sel.format.setBackground( _dbg_line_bgcolor );
        sel.cursor = editor->textCursor();
        sel.cursor.setPosition( block.position() );

    }

    highlight_brackets( editor );

}


void ExtraSelections::highlight_brackets(QPlainTextEdit *editor ){


#define next_char(cur) ((cur).movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor ))
#define sel_next_char(cur) ((cur).movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor ))
#define prev_char(cur) ((cur).movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor ))
#define sel_prev_char(cur) ((cur).movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor ))

#define next_block(cur) ((cur).movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor ))
#define sel_next_block(cur) ((cur).movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor ))
#define prev_block(cur) ((cur).movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor ))
#define sel_prev_block(cur) ((cur).movePosition(QTextCursor::PreviousBlock, QTextCursor::KeepAnchor ))


    _brackets.clear();

    if ( !editor )
        return;



    QTextCursor tc = editor->textCursor();

    bool forward = false;
    QChar chr = 0, chr_to_match = 0;
    int pos = tc.positionInBlock();

    QColor color, fgcolor;
    color.setNamedColor("#cc8f9c");
    fgcolor.setNamedColor("#ffffff");

    QTextEdit::ExtraSelection sel;
    sel.format.setBackground( color );
    sel.format.setForeground( fgcolor );

    auto text = tc.block().text();
    if ( text.size()== 0 )
        return;

    if ( pos == text.size() ) {

        pos = pos -1;
        if ( is_bracket( text[pos]) ){

            sel.cursor = tc;
            prev_char( sel.cursor );
            sel_next_char( sel.cursor);

            chr = text[pos];
            chr_to_match = get_chr_to_match(chr);
            forward = is_open_bracket(chr);

            pos += forward ? 1 : -1;

        }else
            return;


    } else {

        if ( is_open_bracket( text[pos] ) ){

            sel.cursor = tc;
            sel_next_char( sel.cursor);

            chr = text[pos];
            chr_to_match = get_chr_to_match(chr);
            forward = true;

            pos +=1;

        }else if ( pos -1 >= 0  && is_close_bracket( text[pos-1] )  ){

            pos = pos -1 ;

            sel.cursor = tc;
            prev_char( sel.cursor );
            sel_next_char( sel.cursor);

            chr = text[pos];
            chr_to_match = get_chr_to_match(chr);
            forward = is_open_bracket(chr);

            pos -=1;

        } else
            return ;


    }


    if ( chr == 0 || chr_to_match == 0 )
        return;



    // find in block
    int count_inners = 0;

    for ( ; forward ? pos < text.size() : pos >=0 ; pos += forward ? 1:-1  ){

        if ( text[pos] == chr )
            count_inners++;

        if ( text[pos] == chr_to_match ) {
            if ( count_inners == 0 ){


                //qDebug() << "open sel in block at [" << sel.cursor.blockNumber() << "," << sel.cursor.positionInBlock() << "]  -- [" << tc.blockNumber() << ":" << pos << "]";

                QTextEdit::ExtraSelection sel_end;
                sel_end.format.setBackground( color );
                sel_end.format.setForeground( fgcolor );

                sel_end.cursor = tc;
                sel_end.cursor.movePosition( QTextCursor::StartOfBlock );
                sel_end.cursor.movePosition( QTextCursor::NextCharacter , QTextCursor::MoveAnchor , pos );
                sel_next_char( sel_end.cursor );

                _brackets.append( { sel, sel_end});

                return;

            }else
                count_inners--;
        }
    }

    // find in prev/next blocks
    auto* doc = editor->document();
    int nblocks= doc->blockCount();


    // next_block and prev_block don't have end position
    if ( forward && tc.blockNumber() == nblocks-1 )
        return;
    if ( !forward && tc.blockNumber() == 0 )
        return;

    do {

        forward ? next_block( tc ) : prev_block( tc );

        text = tc.block().text();
        for ( int icol = forward ? 0 : text.size()-1; forward ? icol < text.size() : icol >= 0 ; forward ? ++icol : --icol ){

            if ( text [ icol ] == chr )
                count_inners++;

            if ( text[icol] == chr_to_match ) {
                if ( count_inners == 0 ) {


                    //qDebug() << "open sel at [" << sel.cursor.blockNumber() << "," << sel.cursor.positionInBlock() << "]  -- [" << tc.blockNumber() << ":" << icol << "]";


                    QTextEdit::ExtraSelection sel_end;
                    sel_end.format.setBackground( color );
                    sel_end.format.setForeground( fgcolor );

                    sel_end.cursor = tc;
                    sel_end.cursor.movePosition( QTextCursor::StartOfBlock );
                    sel_end.cursor.movePosition( QTextCursor::NextCharacter , QTextCursor::MoveAnchor , icol );
                    sel_next_char( sel_end.cursor );

                    _brackets.append( { sel, sel_end});

                    return;

                }else
                    count_inners--;
            }
        }


        // next_block and prev_block don't have end position
    } while( !( ( forward && tc.blockNumber() == nblocks-1 ) || ( !forward && tc.blockNumber() == 0 ) ) );



#undef next_char
#undef sel_next_char
#undef prev_char
#undef sel_prev_char

#undef next_block
#undef sel_next_block
#undef prev_block
#undef sel_prev_block

}





} // text_editor
