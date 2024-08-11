#ifndef PYTHONSYNTAXHIGHLIGHTER_H
#define PYTHONSYNTAXHIGHLIGHTER_H



#include <QSyntaxHighlighter>
#include "isyntax_highlight.h"



namespace text_editor {




class SyntaxHighlighter : public QSyntaxHighlighter , public IStyler{



public:



    explicit SyntaxHighlighter( QTextDocument *document = nullptr ) ;
    virtual ~SyntaxHighlighter();

    void set_highlighter( ISyntaxHighighter* value);

    void set_state( int value );

    int prev_state( );

    void setFormat( int pos , int n , const QString type_name );

protected:



    void highlightBlock(const QString &text) override ;

    QTextCharFormat format( const QString& color_name, const QString& style="") ;



private :

    ISyntaxHighighter* _highlighter;
};


} // text_editor

#endif // PYTHONSYNTAXHIGHLIGHTER_H
