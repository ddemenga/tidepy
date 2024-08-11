#include <QMouseEvent>
#include <QTextBlock>
#include <QDebug>
#include <QTextStream>
#include <QRegularExpression>
#include <QFile>



#include "search_result.h"



namespace {

void replace( const QString& file , const QString& pattern , const QString& value , bool sensitivity ){

    QRegularExpression exp(pattern , sensitivity ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);


    QFile in(file);

    QString text;
    if ( in.open(QIODevice::ReadOnly )){
        QTextStream ts(&in);

        text = ts.readAll();
        text.replace( exp , value );

        int i =1;
        for ( auto s : text.split('\n') )
            qDebug() << i++ << ">> " << s;

        in.close();
    }


    if ( in.open(QIODevice::ReadWrite )){

        QTextStream ts(&in);
        ts << text;

        in.close();
    }


}


} //



namespace widgets {




TESearchResult::TESearchResult(QWidget *parent)
    : QPlainTextEdit{parent}
{
    setMouseTracking(true);
}


void TESearchResult::mouseMoveEvent(QMouseEvent*e) {


    QTextCursor tc = cursorForPosition( e->pos() );
    if ( tc.block().isValid() && tc.block().length()>1 ){

        QTextEdit::ExtraSelection sel ;
        sel.format.setProperty( QTextFormat::FullWidthSelection, true);
        sel.format.setBackground( QColor(200,200,200) );
        sel.cursor = tc;
        setExtraSelections( { sel } );
    }
}


void TESearchResult::mouseDoubleClickEvent(QMouseEvent*e) {

    QTextCursor tc = cursorForPosition( e->pos() );
    if ( tc.block().isValid() && tc.block().length()>1 ){
        //qDebug() << "Selected " << tc.block().text();
        auto text = tc.block().text();
        int pos = text.indexOf(":");
        int num_pos = text.indexOf(":",pos+1);
        QString num= text.mid( pos+1, num_pos-pos-2);
        emit file_selected( text.left(pos-1).trimmed() , num.toInt() );

    }
}


void TESearchResult::replace_all( const QString& pattern , const QString& value , bool sensitivity  ) {


    QStringList done;
    for (QTextBlock block = document()->begin(); block != document()->end(); block = block.next() ){

        if ( block.blockNumber() == 0 )
            continue;

        if ( block.isValid() && block.length() >1 ){
            //qDebug() << "Selected " << tc.block().text();
            auto text = block.text();
            int pos = text.indexOf(":");
            int num_pos = text.indexOf(":",pos+1);
            QString num= text.mid( pos+1, num_pos-pos-2);

            QString filename = text.left(pos-1).trimmed();
            qDebug() << sensitivity << ":" <<  pattern << ":" << value << ":" << filename  << ":" <<  num.toInt();

            if ( !done.contains(filename) ) {
                replace( text.left(pos-1).trimmed() , pattern , value , sensitivity );
                emit file_changed( filename );
            }
            done.append(filename  );
        }
    }
}


} //widgets


