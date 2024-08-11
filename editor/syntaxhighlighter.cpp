#include <QDebug>
#include <QTextDocument>

#include "syntaxhighlighter.h"



namespace text_editor {


SyntaxHighlighter::SyntaxHighlighter( QTextDocument *document ) : QSyntaxHighlighter(document) , _highlighter(nullptr) {


}

SyntaxHighlighter::~SyntaxHighlighter() {

}

void SyntaxHighlighter::set_highlighter( ISyntaxHighighter* value){
    _highlighter = value;
}

void SyntaxHighlighter::set_state( int value ){
    setCurrentBlockState(value);
}

int SyntaxHighlighter::prev_state( ){
    return previousBlockState();
}


void SyntaxHighlighter::setFormat( int pos , int n , const QString type_name ) {

    //qDebug() << "style block " << currentBlock().blockNumber() << " [" << pos << "," << n << "] " << type_name << "  --> |" << currentBlock().text().mid(pos,n);

    QTextCharFormat f;
    if  ( type_name == "keywords"){
        f = format( "#4488ee","bold");

    }else if ( type_name == "cython_keywords"){
        f = format( "#aa3333","bold");

    }else if ( type_name == "parenthesis"){
        f = format("#aa3333","bold");

    }else if ( type_name == "numbers"){
        f = format("sandybrown","bold") ;

    }else if ( type_name == "comment"){
        f = format("#aaaaaa","bold");

    }else if ( type_name == "decoration"){
        f = format("#dd99dd","bold");

    }else if ( type_name == "exceptions") {
        f = format("#801414","bold");

    }else if ( type_name == "builtins") {
        f = format("#4488ee","normal");

    }else if ( type_name == "simple_string_open"){
        f = format("green","");

    }else if ( type_name == "simple_string_closed"){
        f = format("green","");

    }else if ( type_name == "triple_string_open"){
        f = format("green","");

    }else if ( type_name == "triple_string_closed"){
        f = format("green","");
    }

    QSyntaxHighlighter::setFormat( pos, n ,  f );
}





void SyntaxHighlighter::highlightBlock(const QString &text) {

    if (  _highlighter )
        _highlighter->highlight_block(text,this);

}

QTextCharFormat SyntaxHighlighter::format( const QString& color_name, const QString& style) {

    QColor c;
    c.setNamedColor(color_name);

    QTextCharFormat f;
    f.setForeground(c);
    if ( style == "bold" )
        f.setFontWeight(QFont::Bold);
    else if ( style == "italic" )
        f.setFontItalic(true);

    return f;

}





}
