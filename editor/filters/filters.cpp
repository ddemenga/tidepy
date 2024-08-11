#include <QRegExp>
#include <QTextStream>
#include <QStringList>


#include "filters.h"

namespace text_editor {
namespace filters {

Filter::~Filter() {}

void Filter::set_tab_size( int value ) {

    _tab_size = qMax( 1, value );
    _tab_space = QString(_tab_size, QChar(' '));

}


bool LoopOverSequence::run( QTextCursor& tc ) const   {

    QString name = tc.selectedText();
    if ( !name.size() )
        return false;

    tc.removeSelectedText();

    QString indent( tc.positionInBlock() , QChar(' ') );

    tc.insertText( QString("for i,value in enumerate(%3):\n%1%2pass" ).arg(indent).arg(_tab_space).arg(name) );

    return true;
}


bool LoopOverMap::run( QTextCursor& tc ) const   {

    QString name = tc.selectedText();
    if ( !name.size() )
        return false;

    QString indent( tc.positionInBlock() , QChar(' ') );

    tc.insertText( QString("for k,value in %3.items():\n%1%2pass").arg(indent).arg(_tab_space).arg(name));

    return true;
}



bool CaseFilter::run( QTextCursor& tc ) const  {

    QString txt = tc.selectedText();
    tc.removeSelectedText();

    QString indent( tc.positionInBlock() , QChar(' ') );


    int i =0;
    QTextStream in(&txt);
    QString test_name, name ;
    while (  (in >> name).status() != QTextStream::ReadPastEnd  ) {

        if ( i==0 ) {

            test_name  = name;

        } else if ( i == 1 ){
            tc.insertText( QString( "if %1 == %2 : \n%3%4pass").arg( test_name ).arg( name ).arg(indent).arg(_tab_space) );

        }else {

            tc.insertText( QString( "\n%3elif %1 == %2 : \n%3%4pass").arg( test_name ).arg( name ).arg(indent).arg(_tab_space) );
        }

        i++;
    }

    if ( i == 0  )
        tc.insertText( txt );

    return true;

}



bool DefProperty::run( QTextCursor& tc ) const  {

    QString txt = tc.selectedText();
    tc.removeSelectedText();

    QString indent( tc.positionInBlock() , QChar(' ') );


    bool ok = false;
    QTextStream in(&txt);
    QString name ;
    while (  (in >> name).status() != QTextStream::ReadPastEnd  ) {

        tc.insertText( QString("@property\n%2def %1(self):\n%2%3return self._%1\n%2").arg(name).arg(indent).arg(_tab_space) );
        tc.insertText(QString( "@%1.setter\n%2").arg(name).arg(indent));
        tc.insertText(QString("def %1(self, value):\n%2%3self._%1= value\n%2\n%2").arg(name).arg(indent).arg(_tab_space) );

        ok = true;
    }

    if ( !ok )
        tc.insertText( txt );

    return true;
}


bool ClassFilter::run( QTextCursor& tc )  const {

    QString txt = tc.selectedText();
    tc.removeSelectedText();

    QString indent( tc.positionInBlock() , QChar(' ') );

    bool first = true;
    QTextStream in(&txt);
    QString name ;
    while (  (in >> name).status() != QTextStream::ReadPastEnd  )    {

        if ( first ){
            first = false;
            tc.insertText( QString("class %1(object):\n%2").arg(name).arg(indent) );
        }else{
            tc.insertText( QString("\n%2%3def %1(self):\n%2%3%3pass").arg(name).arg(indent).arg(_tab_space) );
        }

    }

    if ( first )
        tc.insertText( txt );

    return true;

}


bool MemberFunctionFilter::run( QTextCursor& tc ) const  {

    QString txt = tc.selectedText();
    tc.removeSelectedText();

    QString indent( tc.positionInBlock() , QChar(' ') );


    bool ok = false;
    QTextStream in(&txt);
    QString name;
    while (  (in >> name).status() != QTextStream::ReadPastEnd  ) {
        ok = true;
        tc.insertText( QString( "def %1(self):\n%2%3pass\n%2").arg( name ).arg(indent).arg(_tab_space) );

    }

    if ( !ok )
        tc.insertText( txt );

    return ok;
}


bool FunctionFilter::run( QTextCursor& tc ) const  {

    QString txt = tc.selectedText();
    tc.removeSelectedText();

    QString indent( tc.positionInBlock() , QChar(' ') );

    bool ok = false;
    QTextStream in(&txt);
    QString name;
    while (  (in >> name).status() != QTextStream::ReadPastEnd ) {
        ok = true;
        tc.insertText( QString( "def %1():\n%2%3pass\n%2").arg( name ).arg(indent).arg(_tab_space) );
    }

    if ( !ok )
        tc.insertText( txt );

    return ok;
}




} // filters
} // test_editor
