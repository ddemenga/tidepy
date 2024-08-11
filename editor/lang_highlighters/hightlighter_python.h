#ifndef PYTHONHIGHTLIGHTER_H
#define PYTHONHIGHTLIGHTER_H

#include <QRegExp>

#include "../grammar_base.hpp"
#include "../grammar_utils.hpp"
#include "../find.hpp"


#include "../isyntax_highlight.h"


namespace text_editor {
namespace highlighter {


namespace {

template< char C>
struct String : peg::seq< peg::one<C>, peg::star<  peg::sor< peg::literal< '\\',C> ,  peg::seq<peg::not_at< peg::one<C>>,peg::any> >> , peg::one<C>> {};


template< char C>
struct FString: peg::seq < peg::one<'f'> , String<C> >  {} ;

template< char C>
struct RString: peg::seq < peg::one<'r'> , String<C> >  {} ;

template< char C>
struct StringTail : peg::star< peg::sor <   peg::literal<'\\',C>, peg::seq < peg::not_at<peg::one<C>>, peg::any> > > { };

template< char C>
struct TripleStringTail : peg::star< peg::sor <   peg::literal<'\\','\"'>, peg::seq < peg::not_at<peg::literal<C,C,C>>, peg::any> > > { };

}


struct PythonHighighter : ISyntaxHighighter {

    static constexpr int NO_STATE = 0;
    static constexpr int TRIPLE_STRING_OPEN = 1;
    static constexpr int TRIPLE_STRING_OPEN_DOUBLE = 2;


    IStyler* styler;
    QRegExp keywords, cython_keywords, parenthesis, numbers, builtins , exceptions , fstring, fundefs, decoration;



    PythonHighighter();
    virtual ~PythonHighighter();

    template< typename Iterator>
    bool match_tail_triple_string( Iterator& ptr , Iterator const& end ){

        auto mark = ptr;
        for( ; ptr < end && SkipUntil<'\"'>::match(ptr,end); ){
            if ( ptr[-1] != '\\' )
                break;
            else ++ptr;
        }

        if ( ptr >= end ){
            ptr = mark;
            return false;
        }

        for  ( ; FindLiteral<'\"','\"','\"'>::match( ptr , end );  ){
            if ( *(ptr -4) != '\\' )
                return true;
            else ++ptr;
        }

        ptr = mark;
        return false;

    }

    template< char C ,  typename Iterator>
    bool highlight_triple_string( int pos ,Iterator& ptr ,Iterator& e , int skip_begin = 3) {

        auto start = ptr;
        ptr += skip_begin; // skip \"\"\" | \'\'\'

        if ( TripleStringTail<C>::match(ptr,e) ){
            // closed triple string
            if ( ptr == e ) {
                styler->setFormat( pos  , e - start , "triple_string_open" );

            } else {
                ptr+=3; // skip closing \"\"\"
                styler->setFormat( pos  , ptr - start , "triple_string_closed" );

                return true;
            }


        }else{
            // open triple string
            styler->setFormat( pos  , e - start , "triple_string_open" );
            ptr = e ;
        }


        return false;
    }

    template< char C>
    void highlight_string( int offset ,const QChar*& ptr , const QChar* e ){

        auto start = ptr;

        bool ok = String<C>::match( ptr, e );

        styler->setFormat( offset , ptr - start , ok ? "simple_string_closed" : "simple_string_open" );



    }

    template< char C>
    void highlight_rstring( int offset ,const QChar*& ptr , const QChar* e ){

        auto start = ptr;

        bool ok = RString<C>::match( ptr, e );
        //++ptr ; // skip 'f'
        //bool ok = String<C>::match( ptr, e );

        if ( ok ) {
            styler->setFormat( offset , ptr - start , "simple_string_closed" );

        }
    }


    template< char C>
    void highlight_fstring( int offset ,const QChar*& ptr , const QChar* e ){

        auto start = ptr;

        bool ok = FString<C>::match( ptr, e );


        if ( ok ) {
            styler->setFormat( offset , ptr - start , "simple_string_closed" );
            // TODO highlight pairs of {} with different color
        }
    }




    void highlight_block ( const QString& text ,IStyler* styler_obj) override;
};

} // syntax_highlighter
} // text_editor

#endif // PYTHONHIGHTLIGHTER_H
