// based on information: http://en.m.wikipedia.org/wiki/ANSI_escape_code
// http://misc.flogisoft.com/bash/tip_colors_and_formatting
// http://invisible-island.net/xterm/ctlseqs/ctlseqs.html

#ifndef _ANSI_ESCAPE_H
#define _ANSI_ESCAPE_H


#include <QObject>
#include <QTextCharFormat>
#include <QStringList>
#include <QPlainTextEdit>
#include <QColor>
#include <QDebug>



class   AnsiEscape : public QObject {


public :

    AnsiEscape();

    AnsiEscape& set_text_edit( QPlainTextEdit*);

    template< bool terminal=false >
    void format_text( const QString  & text ){
        assert( _tedit );

        terminal ? format_terminal_line( text ) : format_line( text );

        _tedit->ensureCursorVisible();

    }

private:


    void parse_color_sequence(int attribute, QListIterator< QString > & i);

    void format_terminal_line( const QString& line );
    void parse_terminal_text( const QString& str , QTextCursor& cur );

    void format_line( const QString& line );
    void parse_text( const QString& str , QTextCursor& cur );


private:

    QPlainTextEdit * _tedit;
    QTextCharFormat _default_fmt, _text_fmt;

    bool _overwrite;
};


#endif //_ANSI_ESCAPE_H



