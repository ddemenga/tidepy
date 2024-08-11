#ifndef ISYNTAX_HIGHLIGHT_H
#define ISYNTAX_HIGHLIGHT_H


#include <QString>




namespace text_editor {


struct IStyler {

    virtual void set_state( int value )=0;

    virtual int prev_state( ) = 0;

    virtual void setFormat( int pos , int n , const QString type_name ) = 0 ;

};


struct ISyntaxHighighter {

    virtual void highlight_block ( const QString& text ,IStyler* styler)=0;

    virtual ~ISyntaxHighighter() = default;
};


} // text_editor

#endif // ISYNTAX_HIGHLIGHT_H
