#include "hightlighter_python.h"


namespace text_editor {
namespace highlighter {

PythonHighighter::PythonHighighter(){

    keywords = QRegExp("(\\b)(False|None|True|and|as|assert|break|class|continue|def|del|elif|else|except|finally|for|from|global|if|import|in|is|lambda|nonlocal|not|or|pass|raise|return|try|while|with|yield|print|self)\\b");
    cython_keywords = QRegExp("(\\b)(cdef|cimport|cpdef)(\\b)");
    parenthesis = QRegExp(R"([\(\){}\[\]])");
    numbers = QRegExp(R"(\b(([0-9]+(\.[0-9]+)?)|(\.[0-9]+))((e|E)(\+|\-)?[0-9]+)?\b)");
    builtins =  QRegExp( "\\b(abs|aiter|all|anext|any|ascii|bin|bool|breakpoint|bytearray|bytes|callable|chr|classmethod|compile|complex|delattr|dict|dir|divmod|enumerate|eval|exec|filter|float|format|frozenset|getattr|globals|hasattr|hash|help|hex|id|input|int|isinstance|issubclass|iter|len|list|locals|map|max|memoryview|min|next|object|oct|open|ord|pow|print|property|range|repr|reversed|round|set|setattr|slice|sorted|staticmethod|str|sum|super|tuple|type|vars|zip)\\b");
    exceptions = QRegExp( "\\b(ArithmeticError|AssertionError|AttributeError|EnvironmentError|EOFError|Exception|FloatingPointError|ImportError|IndentationError|IndexError|IOError|KeyboardInterrupt|KeyError|LookupError|MemoryError|NameError|NotImplementedError|OSError|OverflowError|ReferenceError|RuntimeError|StandardError|StopIteration|SyntaxError|SystemError|SystemExit|TabError|TypeError|UnboundLocalError|UnicodeDecodeError|UnicodeEncodeError|UnicodeError|UnicodeTranslateError|ValueError|WindowsError|ZeroDivisionError|Warning|UserWarning|DeprecationWarning|PendingDeprecationWarning|SyntaxWarning|OverflowWarning|RuntimeWarning|FutureWarning)\\b");
    //fstring = QRegExp( "(f\"([^\"\\\\]*(\\\\.[^\"\\\\]*)*)\")|('([^'\\\\]*(\\\\.[^'\\\\]*)*)')");
    //fundefs = QRegExp("\\s*def\\s*([_\\w][_\\w\\d]*)\\s*\\(");
    //fundefs = QRegExp(R"(\s*def\s*([_\w][_\w\d]*)\s*\()");
    decoration = QRegExp(R"(@\w+(.\w+)*\b)");


}



void PythonHighighter::highlight_block (const QString& text , IStyler *styler_obj)  {

    styler = styler_obj;

    styler->set_state( NO_STATE );

    int pos =0;
    while ((pos = keywords.indexIn(text, pos)) != -1 ) {

        int dpos = keywords.matchedLength();

        int pos_cap = keywords.pos(2);

        assert( dpos > 0 );

        //Debug() << text.mid( pos_cap, dpos - (pos_cap-pos) );

        styler_obj->setFormat( pos_cap, dpos -(pos_cap - pos) , "keywords");

        pos += dpos;

    }

    pos =0;
    while ((pos = cython_keywords.indexIn(text, pos)) != -1 ) {

        int dpos = cython_keywords.matchedLength();

        int pos_cap = cython_keywords.pos(2);

        assert( dpos > 0 );

        //Debug() << text.mid( pos_cap, dpos - (pos_cap-pos) );

        styler_obj->setFormat( pos_cap, dpos -(pos_cap - pos) , "cython_keywords");

        pos += dpos;

    }

    pos =0;
    while ((pos = parenthesis.indexIn(text, pos)) != -1 ) {

        int dpos = parenthesis.matchedLength();

        assert( dpos > 0 );

        //Debug() << text.mid( pos_cap, dpos - (pos_cap-pos) );

        styler_obj->setFormat( pos, dpos , "parenthesis");

        pos += dpos;

    }


    pos =0;
    while ((pos = numbers.indexIn(text, pos)) != -1 ) {

        int dpos = numbers.matchedLength();

        assert( dpos > 0 );

        //Debug() << text.mid( pos_cap, dpos - (pos_cap-pos) );

        styler_obj->setFormat( pos, dpos , "numbers");

        pos += dpos;

    }

    pos =0;
    while ((pos = exceptions.indexIn(text, pos)) != -1 ) {

        int dpos = exceptions.matchedLength();

        assert( dpos > 0 );

        //Debug() << text.mid( pos_cap, dpos - (pos_cap-pos) );

        styler_obj->setFormat( pos, dpos , "exceptions");

        pos += dpos;

    }


    pos =0;
    while ((pos = builtins.indexIn(text, pos)) != -1 ) {

        int dpos = builtins.matchedLength();

        assert( dpos > 0 );

        //Debug() << text.mid( pos_cap, dpos - (pos_cap-pos) );

        styler_obj->setFormat( pos, dpos , "builtins");

        pos += dpos;

    }

    pos =0;
    while ((pos = decoration.indexIn(text, pos)) != -1 ) {

        int dpos = decoration.matchedLength();

        assert( dpos > 0 );

        //Debug() << text.mid( pos_cap, dpos - (pos_cap-pos) );

        styler_obj->setFormat( pos, dpos , "decoration");

        pos += dpos;

    }



    // comments and strings
    {

        const QChar *e = text.data()+text.size() , *ptr = text.data();

        auto prev_state = styler_obj->prev_state();

        if ( prev_state == TRIPLE_STRING_OPEN  || prev_state == TRIPLE_STRING_OPEN_DOUBLE){

            // trying closing the triple string
            if ( prev_state == TRIPLE_STRING_OPEN && highlight_triple_string<'\''>( 0, ptr, e , 0 ) ){
                // triple string closed
                styler_obj->set_state(NO_STATE);


            }else if ( prev_state == TRIPLE_STRING_OPEN_DOUBLE && highlight_triple_string<'\"'>( 0, ptr, e , 0 ) ) {
                // triple string closed
                styler_obj->set_state(NO_STATE);

            }else {
                // triple string still open
                styler_obj->set_state(prev_state);
            }

        }

        for ( auto last = ptr ;  ptr < e ;   last = ptr  ){


            if ( *ptr == '#'){

                int pos  = ptr - text.data();
                styler_obj->setFormat( pos , text.size() - pos , "comment" );
                break;

            }else if ( *ptr == '\"'){

                if ( e-ptr >= 3 && *(ptr+1) == '\"' && *(ptr+2) == '\"' ) {

                   if ( !highlight_triple_string<'\"'>( ptr-text.data(), ptr, e ) ){
                       // triple string not close
                       styler_obj->set_state( TRIPLE_STRING_OPEN_DOUBLE );
                       break;
                   }

                }else
                    highlight_string<'\"'>( ptr-text.data(), ptr, e );


            }else if ( *ptr == '\'') {

                if ( e-ptr >= 3 && *(ptr+1) == '\'' && *(ptr+2) == '\'' ) {

                   if ( !highlight_triple_string<'\''>( ptr-text.data(), ptr, e ) ){
                       // triple string not close
                       styler_obj->set_state( TRIPLE_STRING_OPEN );
                       break;
                   }

                }else
                    highlight_string<'\''>(ptr-text.data(),ptr, e );

            }else if ( *ptr == 'f') {

                if ( e-ptr >= 2 && ptr[1]=='\"' )
                    highlight_fstring<'\"'>(ptr-text.data(),ptr, e );
                else  if ( e-ptr >= 2 && ptr[1]=='\'' )
                    highlight_fstring<'\''>(ptr-text.data(),ptr, e );

            }else if ( *ptr == 'r') {

                if ( e-ptr >= 2 && ptr[1]=='\"' )
                    highlight_rstring<'\"'>(ptr-text.data(),ptr, e );
                else  if ( e-ptr >= 2 && ptr[1]=='\'' )
                    highlight_rstring<'\''>(ptr-text.data(),ptr, e );
            }


            if ( last == ptr ){
                // no match found ? go 1 char forward
                ++ptr ;
            }
        }
    }

}


PythonHighighter::~PythonHighighter() {}

}
}
