#ifndef FIND_HPP
#define FIND_HPP


template< char C >
struct SkipUntil {

    template<typename Iterator>
    static bool match( Iterator& s, Iterator const&  e){

        if ( s == e )
            return false;

        for ( ; s != e && ( *s != C ) ; ){
            ++s;
        }

        if ( s==e && *s != C )
            return false;

        return true;
    }
};

template< char ... Cs >
struct FindLiteral {

    template<typename Iterator>
    static bool match( Iterator& s, Iterator const&  e){


        auto it = s;
        if ( ! ( ( match<Cs,Iterator>(s,e)) && ... ) ){
            s= it;
            return false;
        }
        return true;
    }

    template<char C , typename Iterator>
    static bool match( Iterator& s, Iterator const& e ){
        bool tmp = s!=e && *s == C;
        if ( tmp ) ++s;
        return tmp;
    }

};



#endif // FIND_HPP
