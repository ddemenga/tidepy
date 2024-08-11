#ifndef EXTRASELECTIONS_H
#define EXTRASELECTIONS_H

#include <QObject>
#include <QPlainTextEdit>
#include <QList>




namespace text_editor {

class ExtraSelections : public QObject {

    Q_OBJECT

public:

    explicit ExtraSelections(QObject *parent = nullptr);

    void clear();
    void highlight(QPlainTextEdit * );


    const QList<QTextEdit::ExtraSelection>& lines_selections() const ;
    const QList<QTextEdit::ExtraSelection>& brackets_selections() const ;


public slots:

    void set_current_line_bgcolor(const QBrush & );
    void set_debug_line_bgcolor(const QBrush & );

    void set_debug_line( unsigned line );

private:

    void highlight_brackets( QPlainTextEdit* );

    inline bool is_open_bracket( QChar chr ) {
        return chr == '{' || chr == '[' || chr =='(';
    }

    inline bool is_close_bracket( QChar chr ) {
        return chr == '}' || chr == ']' || chr ==')';
    }

    inline bool is_bracket( QChar chr ){
        return is_open_bracket(chr) || is_close_bracket(chr);
    }

    inline QChar get_chr_to_match( QChar chr ) {
        if ( is_open_bracket(chr) )
            return chr =='{' ? '}' : ( chr=='[' ? ']' : ')' );
        else
            return chr =='}' ? '{' : ( chr==']' ? '[' : '(' );
    }

private:

    QList<QTextEdit::ExtraSelection> _lines;
    int _debug_line = -1;

    QList<QTextEdit::ExtraSelection> _brackets;

    QBrush _current_line_bgcolor, _dbg_line_bgcolor;

};


} // text_editor


#endif // EXTRASELECTIONS_H
