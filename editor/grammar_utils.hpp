#ifndef GRAMMAR_UTILS_HPP
#define GRAMMAR_UTILS_HPP


#include <iterator>

#include "grammar_base.hpp"




namespace  peg {


// Careful : not consumes the current pos !
struct eof : Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& s, Iterator& e ,States&... ){

        if  (  s==e  ) { return true; }
        return false;
    }
};

struct any :Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& s, Iterator& e ,States&... ){

        if  (  s!=e &&  *s != 0  ) { ++s; return true; }
        return false;
    }
};


template<char C>
struct one : Base  {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& s, Iterator& e ,States&... ){

        if  (  s != e && ( *s == C ) ) { ++s;  return true; }

        return false;
    }

};


template<char ... Cs>
struct one_of : Base  {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& s, Iterator& e ,States&... ){

        auto it = s;
        if ( ! ( ( match<Cs,Iterator>(s,e)) || ... ) ){
            s= it;
            return false;
        }
        return true;
    }

    template<char Char , typename Iterator>
    static bool match( Iterator& s, Iterator& e ){
        bool tmp = s!=e && *s == Char;
        if ( tmp ) ++s;
        return tmp;
    }

};


template< char ... Cs >
struct literal :Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& s, Iterator&  e,States&... ){

        auto it = s;
        if ( ! ( ( match<Cs,Iterator>(s,e)) && ... ) ){
            s= it;
            return false;
        }
        return true;
    }

    template<char Char , typename Iterator>
    static bool match( Iterator& s, Iterator& e ){
        bool tmp = s!=e && *s == Char;
        if ( tmp ) ++s;
        return tmp;
    }

};


template< char L, char R >
struct range : Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& s, Iterator&  e ,States&... ){

        if ( s != e && ( L <= *s ) && ( *s <= R )) { ++s; return true; }
        return false;

    }

};

struct match_name : Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& s, Iterator& e ,States&... ){

        if ( s == e) return false;

        auto mark = s;

        if ( !isalpha(*s) && (*s!='_') ){

            return false;
        }

        ++s;

        int i = 1;
        for( ; s != e && ( isalnum(*s) || *s == '_' ) ; ++s,++i );

        if ( i ){
            return true;
        }

        s = mark;

        return false;

    }

};


struct match_identifier : Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& s, Iterator& e ,States&... ){

        if ( s == e) return false;

        auto mark = s;

        if ( !isalpha(*s) && (*s!='_') ){

            return false;
        }

        ++s;

        int i = 1;
        for( ; s != e && ( ( isalnum(*s) || *s == '_' )  && *s != '.' ) ; ++s,++i );

        if ( i ){
            return true;
        }

        s = mark;

        return false;

    }

};



/**
 * Match a string : ( "'" ( !"'" _char )* "'" ) | ( '"' ( !'"' _char )* '"'  )
 *
 * @brief The match_id struct
**/
struct match_string : Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& it, Iterator&  end ,States&... ){


        if ( it == end   ) return false;

        auto mark_pos = it;

        // ( "'" ( !"'" _char )* "'" ) | ( '"' ( !'"' _char )* '"'  )
        if ( (*it != '\'' ) &&  ( *it != '\"')  ){

            return false;
        }

        const auto marker = *it;
        auto last_value = marker;

        ++it;

        for( ; it != end && (*it) ;  ++it ){
            //if ( (*it != marker ) || ( ( *it == marker) && (last_value=='\\') ) )
            if ( (*it != marker ) || (last_value=='\\') )
                last_value = *it;
            else break;
        }


        if ( it !=end && (*it == marker) && last_value != '\\' ){
            ++it;

            return true;
        }


        it = mark_pos;

        return false;

    }

};


/**
 * Match a number : ('+' | '-' )?  ( ( [0-9]+  ( '.' [0-9]+)? ) |  ( '.' [0-9]+ )  )  ( ( 'e' | 'E' ) ( '+' | '-' )?  [0-9]+ )?
 * @brief The match_float struct
**/
struct match_number  : Base{

    using need_enter_type = match_number;


    template<typename Iterator,typename ...States>
    static bool match(  Iterator& it, Iterator&  end ,States&... ){

        if ( it == end ) return false;

        auto mark = it;

        unsigned i=0;

        // ('+' | '-' )?
        if ( (*it == '+') || (*it == '-') ){
            ++it; ++i;

            if ( it == end && i ){
                it = mark;
                return false;

            }
        }

        if ( std::isdigit(*it)  ){
            // [0-9]+ ( '.' [0-9]+ )?

            if ( it != end &&  !std::isdigit(*it) ) {
                it = mark;
                return false;
            }
            while ( it != end && std::isdigit(*it) ) { ++i; ++it; }


            auto mark2 = it;
            if ( it != end && *it == '.' ) {

                ++i; ++it;

                if ( it == end || !std::isdigit( *it ) ) {
                    it = mark2; i-=2;

                }else {

                    ++it; ++i;
                    while ( it != end && std::isdigit(*it) ) { ++i;++it;}

                }
            }

        } else if ( *it == '.' ){
            // ( '.' [0-9]+ )

            if ( it != end && *it != '.' )  {

                it  = mark;
                return false;
            }

            ++i; ++it;

            if ( it == end ||  !std::isdigit(*it) ) {

                it = mark;
                return false;
            }
            while ( it != end && std::isdigit(*it) ) { ++i; ++it; }



        } else {
            // input is not '.' | [0-9]

            it = mark;


            return false;

        }


        //( ( 'e' | 'E' ) ( '+' | '-' )?  [0-9]+ )?
        if ( ( it != end) && ( (*it == 'e') || (*it == 'E') ) ) {
            ++i; ++it ;

            if ( (it != end) && ( (*it == '+') || (*it == '-') ) ) { ++i; ++it ; }

            if ( it == end ||  !std::isdigit(*it) ) {
                it = mark;

                return false;
            }
            while ( it != end && std::isdigit(*it) ) { ++i; ++it; }
        }




        return true;
    }

};



template<char ... Cs>
struct match_c_comment : Base {

    template<typename Iterator,typename ...States>
    static bool match( Iterator& it, Iterator&  end ,States&... s){

        if ( it == end ) return false;

        if ( ! literal<Cs...>::match(  it, end, s... )  ){
            return false;
        }

        if ( !it.skip_space ){

            for( ; !it.is_eol(); ++it );


        }else {

            auto pos = it.pos();

            for( ; it.pos().line == pos.line; ++it );


        }



        return true;

    }

};

/**
 * Match a triple string :  ( '"""'  _char* '"""'  )
 *
 * @brief The match_triple_string struct
 */
struct match_triple_string : Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& it, Iterator&  end ,States&... ){


        if ( it == end ) return false;


        if ( (end - it ) < 6 )
            return false;


        auto marker_3 = *(it+2), marker_2 = *(it+1);

        // ( "'" ( !"'" _char )* "'" ) | ( '"' ( !'"' _char )* '"'  )
        if ( (marker_3 != '\"' ) ||  ( marker_2 != '\"') || ( *it!= '\"' )  )
            return false;

        auto mark = it;

        std::advance( it , 3 );

        marker_3 = marker_2 = 0;

        for( ; it != end && (*it) ;  ++it  ){

            if ( (*it != '\"' ) ||  marker_2 != '\"' || marker_3 != '\"'  ){
                marker_3 = marker_2;
                marker_2 = *it;
            }
            else break;


        }


        if ( it != end && (*it == '\"') && marker_2=='\"' && marker_3=='\"'  ){
            ++it;

            return true;
        }



        it = mark;

        return false;

    }
};



/**
 * Special operator :  "\r\n" | '\r' | '\n'
 *
 * @brief The match_eol struct
 */
struct match_eol : Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& it, Iterator&  end ,States&... ){



        if ( it == end ){

            return false;
        }

        auto mark = it;

        if ( *it == '\r'  ) {

            ++it;

            if ( it != end && *it == '\n'){
                ++it;

            }


            return true;

        } else if ( *it == '\n' ){

            ++it;

            return true;
        }

        it = mark;


        return false;

    }

};


/**
 * Special operator :  (!eol . )*
 *
 * @brief The match_eol struct
 */
struct match_until_eol : Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& it, Iterator&  end ,States&... states  ){



        if ( it == end ){

            return true;
        }


        // consume until the next eol
        for( ; !match_eol::match(it,end,states...) ; ++it );

        // consume all the consecutive eol
        for( ; match_eol::match(it,end,states...); );


        return true;

    }

};








/**
 * comment : @C (!eol .)* eol ;
 *
 * @brief The match_comment struct
 */
template < char C>
struct match_comment : Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& it, Iterator&  end ,States&... states ){


        if ( it == end ){

            return false;
        }



        if (  *it != C ){

            return false;
        }


        for(++it ; it != end && !match_eol::match(it,end,states...) ; ++it  );


        return true;


    }

};

/**
 * space : ' ' | '\t' | eol ;
 *
 * @brief The match_single_space struct
 */
struct match_single_space : Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& it, Iterator&  end ,States&... states){

        if ( it == end ){

            return false;
        }

        if ( !match_eol::match(it,end, states...) ){
            if (  (*it != ' ') && ( *it != '\t')  )
                return false;
            else ++it;
        }

        return true;


    }

};



/**
 * space : ( ' ' | '\t' | eol )+  [ but not empty set ];
 *
 * @brief The match_space struct
 */
struct match_space : Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& it, Iterator&  end ,States&... states){

        if ( it == end ){

            return false;
        }

        if ( !match_eol::match(it,end, states...) ){
            if (  (*it != ' ') && ( *it != '\t')  )
                return false;
            else ++it;
        }


        for( ; it != end ;   ){
            if  ( (*it==' ') || ( *it == '\t' ) ) ++it;
            else if ( !match_eol::match(it,end,states...) ) break;
        }


        return true;


    }

};



struct error_guard :Base {
    using error = std::true_type;

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& s, Iterator& e ,States&... ){

        if  (  s!=e &&  *s  ) { return true; }
        return false;
    }
};






/**
 * Match define : match  everything inside a pair of brace {  <matched> }
 *
 * @brief The match_id struct
 */
struct match_define : Base {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& it, Iterator&  end ,States&... ){

        if ( it == end   ) return false;

        auto mark_pos = it;

        // ( "'" ( !"'" _char )* "'" ) | ( '"' ( !'"' _char )* '"'  )
        if ( *it != '{' )
            return false;

        //const auto marker = *it;


        ++it;


        for( int brace_count = 1; it != end && (*it) ;  ++it ){
            if ( *it != '}' || brace_count )
                continue;
            else if  ( *it == '{' )
                brace_count++;
            else if ( *it == '}' ){
                brace_count--;
                if ( brace_count == 0 )
                    return true;
            }
        }


        it = mark_pos;

        return false;



    }

};



template< char C ,  int N = 1   >
struct check_ahead : Base {


    template<typename Iterator,typename ...States>
    static bool match(  Iterator& it, Iterator&  end ,States&... ){

        if ( it == end && C != 0  )
            return false;

        auto mark = it;


        std::advance( it , N );

        if ( it < end && *it == C ){
            it = mark;
            return true;
        }

        it = mark;
        return false;

    }
};

template< int N , typename ... Rules>
struct check_ahead_one : sor <Rules...>  {

    template<typename Iterator,typename ...States>
    static bool match(  Iterator& s, Iterator& e ,States&...states ){

        if ( s == e  )
            return false;

        auto mark = s;


        std::advance( s , N );

        if (  sor<Rules...>::match(  s,e,states...) ){
            s= mark;
            return true;
        }
        s = mark;
        return false;
    }



};

template< typename R >
struct until : star< seq< not_at<R>, any > > {};

template< char...Cs>
struct keyword : seq< literal<Cs...>, match_space>{};

template< typename A, typename B >
struct list_sep : star<seq<A,B>> { };

template< typename ... Ts >
struct star_seq : star< seq< Ts...>> {};

template< typename ... Ts >
struct star_sor : star< sor< Ts...>> {};

template< typename ... Ts >
struct plus_seq : plus< seq< Ts...>> {};

template< typename ... Ts >
struct plus_sor : plus< sor< Ts...>> {};

template< typename ... Ts >
struct opt_seq : opt< seq< Ts...>> {};










} // peg





#endif // GRAMMAR_UTILS_HPP
