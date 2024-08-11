#ifndef GRAMMAR_BASE_H
#define GRAMMAR_BASE_H

#include <type_traits>
#include <cassert>
#include <map>
#include <stddef.h>


namespace  peg  {



// == Exception handling ======================================================================

struct error_handler {

    template<typename Ast,typename Iterator,typename ...States>
    static void open( Ast& , Iterator&  , Iterator&  , States&...){

    }

    template<typename Ast,typename Iterator,typename ...States>
    static void success( Ast& , Iterator&  , Iterator&  , States&...){

    }

    template<typename Ast,typename Iterator,typename ...States>
    static void failed( Ast& , Iterator&  , Iterator&  , States&...){

    }

};


template<typename Rule>
struct error_selector {
    using type = error_handler;
};


/**template<PegUnit Unit>
struct base_error_selector {
    using type = error_handler;
};**/


// == Ast Manipulation ===========================================================






// == Base objects ===========================================================



struct Base {


    template<typename Iterator,typename ...States>
    static void enter( Iterator& , Iterator&  , States&...){

    }

    template<typename Iterator,typename ...States>
    static void success( Iterator& , Iterator&  , States&...){

    }

    template<typename Iterator,typename ...States>
    static void failed( Iterator& , Iterator&  , States&...){

    }
};



struct Container : Base {


    template< typename Rule , typename Iterator,typename ...States>
    static bool __apply_rule__(Iterator& s, Iterator& e,States&...states ){


        if ( Rule::match(s,e,states...) ) {
            return true;
        }
        return false;
    }

};






// == Base Parser Units ===============================================



template< typename ... Ts >
struct seq : Container {

    template< typename Iterator, typename ...States>
    static bool match( Iterator& s, Iterator& e ,States&... states){




        auto mark = s;

        bool ret = true;
        ( ( ret &= __apply_rule__<Ts,Iterator,States...>(s,e,states...)) && ... );

        if ( !ret ) {
            s = mark;
            return ret;
        }

        return ret;


    }


};




template< typename ... Ts >
struct sor : Container {

    template<typename Iterator, typename ...States>
    static bool match(Iterator& s, Iterator& e ,States&... states){

        return  ( ( __apply_rule__<Ts,Iterator,States...>(s,e,states...)) || ... );


    }

};


template< typename Rule >
struct star : Container {

    template<typename Iterator, typename ...States>
    static bool match( Iterator& s, Iterator& e ,States&... states){


        for( auto watch_dog = s;  __apply_rule__<Rule,Iterator,States...>( s,e,states...); watch_dog = s )
            assert( watch_dog != s );

        return true;
    }



};

template< typename Rule >
struct plus : Container {

    template< typename Iterator, typename ...States>
    static bool match( Iterator& s, Iterator& e ,States&... states){

        if ( s==e ) return false;

        if (!__apply_rule__<Rule,Iterator,States...>(s,e,states...)) return false;


        for( auto watch_dog = s;  __apply_rule__<Rule,Iterator,States...>(s,e,states...) ; watch_dog = s  ){
            assert( watch_dog != s );
        }
        return true;
    }

};




template< typename Rule >
struct opt : Container {

    template< typename Iterator, typename ...States>
    static bool match( Iterator& s, Iterator& e ,States&... states){

        if ( s==e) return true;

        __apply_rule__<Rule,Iterator,States...>(s,e,states...);
        return true;
    }

};





template< typename T >
struct not_at : Base {

    template< typename Iterator, typename ...States>
    static bool match( Iterator& s, Iterator& e ,States&... states){

        auto tmp = s;

        auto ret = T::template match<Iterator,States...>(s,e,states...);
        if ( !ret ){

            return true;
        }


        s=tmp;
        return false;
    }

};

template< typename T >
struct at : Base {

    template< typename Iterator, typename ...States>
    static bool match( Iterator& s, Iterator& e ,States&... states){



        auto tmp = s;




        auto ret = T::template match<Iterator,States...>(s,e,states...);
        if ( ret ){

            s = tmp;
            return true;
        }


        return false;
    }


};







} // end namespace




#endif // GRAMMAR_BASE_H
