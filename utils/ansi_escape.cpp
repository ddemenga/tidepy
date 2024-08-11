#include <QTextBlock>

#include "utils/ansi_escape.h"


AnsiEscape::AnsiEscape() : _tedit(nullptr) {
    _overwrite = false;
}

AnsiEscape& AnsiEscape::set_text_edit( QPlainTextEdit* edit ) {
    _tedit = edit;

    QTextCursor cursor = _tedit->textCursor();
    _default_fmt = cursor.charFormat();
    _text_fmt = _default_fmt;

    return *this;
}

void AnsiEscape::parse_color_sequence(int attribute, QListIterator< QString > & i )
{
    switch (attribute) {
    case 0 : { // Normal/Default (reset all attributes)
        _text_fmt = _default_fmt;
        break;
    }
    case 1 : { // Bold/Bright (bold or increased intensity)
        _text_fmt.setFontWeight(QFont::Bold);
        break;
    }
    case 2 : { // Dim/Faint (decreased intensity)
        _text_fmt.setFontWeight(QFont::Light);
        break;
    }
    case 3 : { // Italicized (italic on)
        _text_fmt.setFontItalic(true);
        break;
    }
    case 4 : { // Underscore (single underlined)
        _text_fmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        _text_fmt.setFontUnderline(true);
        break;
    }
    case 5 : { // Blink (slow, appears as Bold)
        _text_fmt.setFontWeight(QFont::Bold);
        break;
    }
    case 6 : { // Blink (rapid, appears as very Bold)
        _text_fmt.setFontWeight(QFont::Black);
        break;
    }
    case 7 : { // Reverse/Inverse (swap foreground and background)
        QBrush foregroundBrush = _text_fmt.foreground();
        _text_fmt.setForeground(_text_fmt.background());
        _text_fmt.setBackground(foregroundBrush);
        break;
    }
    case 8 : { // Concealed/Hidden/Invisible (usefull for passwords)
        _text_fmt.setForeground(_text_fmt.background());
        break;
    }
    case 9 : { // Crossed-out characters
        _text_fmt.setFontStrikeOut(true);
        break;
    }
    case 10 : { // Primary (default) font
        _text_fmt.setFont(_default_fmt.font());
        break;
    }
    case 11 ... 19 : {
        QFontDatabase fontDatabase;
        QString fontFamily = _text_fmt.fontFamily();
        QStringList fontStyles = fontDatabase.styles(fontFamily);
        int fontStyleIndex = attribute - 11;
        if (fontStyleIndex < fontStyles.length()) {
            _text_fmt.setFont(fontDatabase.font(fontFamily, fontStyles.at(fontStyleIndex), _text_fmt.font().pointSize()));
        }
        break;
    }
    case 20 : { // Fraktur (unsupported)
        break;
    }
    case 21 : { // Set Bold off
        _text_fmt.setFontWeight(QFont::Normal);
        break;
    }
    case 22 : { // Set Dim off
        _text_fmt.setFontWeight(QFont::Normal);
        break;
    }
    case 23 : { // Unset italic and unset fraktur
        _text_fmt.setFontItalic(false);
        break;
    }
    case 24 : { // Unset underlining
        _text_fmt.setUnderlineStyle(QTextCharFormat::NoUnderline);
        _text_fmt.setFontUnderline(false);
        break;
    }
    case 25 : { // Unset Blink/Bold
        _text_fmt.setFontWeight(QFont::Normal);
        break;
    }
    case 26 : { // Reserved
        break;
    }
    case 27 : { // Positive (non-inverted)
        QBrush backgroundBrush = _text_fmt.background();
        _text_fmt.setBackground(_text_fmt.foreground());
        _text_fmt.setForeground(backgroundBrush);
        break;
    }
    case 28 : {
        _text_fmt.setForeground(_default_fmt.foreground());
        _text_fmt.setBackground(_default_fmt.background());
        break;
    }
    case 29 : {
        _text_fmt.setUnderlineStyle(QTextCharFormat::NoUnderline);
        _text_fmt.setFontUnderline(false);
        break;
    }
    case 30 ... 37 : {
        int colorIndex = attribute - 30;
        QColor color;
        if (QFont::Normal < _text_fmt.fontWeight()) {

            switch (colorIndex) {
            case 0 : {
                color = Qt::darkGray;
                break;
            }
            case 1 : {
                color = QColor(240, 82, 79);
                break;
            }
            case 2 : {
                color = QColor(98, 177, 32);
                break;
            }
            case 3 : {
                color = QColor(166, 138, 13);
                break;
            }
            case 4 : {
                color = QColor(57, 147, 212);
                break;
            }
            case 5 : {
                color = QColor(167, 113, 191);
                break;
            }
            case 6 : {
                color = QColor(0, 163, 163);
                break;
            }
            case 7 : {
                color = Qt::white;
                break;
            }
            default : {
                Q_ASSERT(false);
            }
            }
        } else {
            switch (colorIndex) {
            case 0 : {
                color = Qt::black;
                break;
            }
            case 1 : {
                color = Qt::darkRed;
                break;
            }
            case 2 : {
                color = Qt::darkGreen;
                break;
            }
            case 3 : {
                color = Qt::darkYellow;
                break;
            }
            case 4 : {
                color = Qt::darkBlue;
                break;
            }
            case 5 : {
                color = Qt::darkMagenta;
                break;
            }
            case 6 : {
                color = Qt::darkCyan;
                break;
            }
            case 7 : {
                color = Qt::lightGray;
                break;
            }
            default : {
                Q_ASSERT(false);
            }
            }
        }
        _text_fmt.setForeground(color);
        break;
    }
    case 38 : {
        if (i.hasNext()) {
            bool ok = false;
            int selector = i.next().toInt(&ok);
            Q_ASSERT(ok);
            QColor color;
            switch (selector) {
            case 2 : {
                if (!i.hasNext()) {
                    break;
                }
                int red = i.next().toInt(&ok);
                Q_ASSERT(ok);
                if (!i.hasNext()) {
                    break;
                }
                int green = i.next().toInt(&ok);
                Q_ASSERT(ok);
                if (!i.hasNext()) {
                    break;
                }
                int blue = i.next().toInt(&ok);
                Q_ASSERT(ok);
                color.setRgb(red, green, blue);
                break;
            }
            case 5 : {
                if (!i.hasNext()) {
                    break;
                }
                int index = i.next().toInt(&ok);
                Q_ASSERT(ok);
                switch (index) {
                case 0x00 ... 0x07 : { // 0x00-0x07:  standard colors (as in ESC [ 30..37 m)
                    return parse_color_sequence(index - 0x00 + 30, i);
                }
                case 0x08 ... 0x0F : { // 0x08-0x0F:  high intensity colors (as in ESC [ 90..97 m)
                    return parse_color_sequence(index - 0x08 + 90, i);
                }
                case 0x10 ... 0xE7 : { // 0x10-0xE7:  6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
                    index -= 0x10;
                    int red = index % 6;
                    index /= 6;
                    int green = index % 6;
                    index /= 6;
                    int blue = index % 6;
                    index /= 6;
                    Q_ASSERT(index == 0);
                    color.setRgb(red, green, blue);
                    break;
                }
                case 0xE8 ... 0xFF : { // 0xE8-0xFF:  grayscale from black to white in 24 steps
                    qreal intensity = qreal(index - 0xE8) / (0xFF - 0xE8);
                    color.setRgbF(intensity, intensity, intensity);
                    break;
                }
                }
                _text_fmt.setForeground(color);
                break;
            }
            default : {
                break;
            }
            }
        }
        break;
    }
    case 39 : {
        _text_fmt.setForeground(_default_fmt.foreground());
        break;
    }
    case 40 ... 47 : {
        int colorIndex = attribute - 40;
        QColor color;
        switch (colorIndex) {
        case 0 : {
            color = Qt::darkGray;
            break;
        }
        case 1 : {
            color = Qt::red;
            break;
        }
        case 2 : {
            color = Qt::green;
            break;
        }
        case 3 : {
            color = Qt::yellow;
            break;
        }
        case 4 : {
            color = Qt::blue;
            break;
        }
        case 5 : {
            color = Qt::magenta;
            break;
        }
        case 6 : {
            color = Qt::cyan;
            break;
        }
        case 7 : {
            color = Qt::white;
            break;
        }
        default : {
            Q_ASSERT(false);
        }
        }
        _text_fmt.setBackground(color);
        break;
    }
    case 48 : {
        if (i.hasNext()) {
            bool ok = false;
            int selector = i.next().toInt(&ok);
            Q_ASSERT(ok);
            QColor color;
            switch (selector) {
            case 2 : {
                if (!i.hasNext()) {
                    break;
                }
                int red = i.next().toInt(&ok);
                Q_ASSERT(ok);
                if (!i.hasNext()) {
                    break;
                }
                int green = i.next().toInt(&ok);
                Q_ASSERT(ok);
                if (!i.hasNext()) {
                    break;
                }
                int blue = i.next().toInt(&ok);
                Q_ASSERT(ok);
                color.setRgb(red, green, blue);
                break;
            }
            case 5 : {
                if (!i.hasNext()) {
                    break;
                }
                int index = i.next().toInt(&ok);
                Q_ASSERT(ok);
                switch (index) {
                case 0x00 ... 0x07 : { // 0x00-0x07:  standard colors (as in ESC [ 40..47 m)
                    return parse_color_sequence(index - 0x00 + 40, i);
                }
                case 0x08 ... 0x0F : { // 0x08-0x0F:  high intensity colors (as in ESC [ 100..107 m)
                    return parse_color_sequence(index - 0x08 + 100, i);
                }
                case 0x10 ... 0xE7 : { // 0x10-0xE7:  6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
                    index -= 0x10;
                    int red = index % 6;
                    index /= 6;
                    int green = index % 6;
                    index /= 6;
                    int blue = index % 6;
                    index /= 6;
                    Q_ASSERT(index == 0);
                    color.setRgb(red, green, blue);
                    break;
                }
                case 0xE8 ... 0xFF : { // 0xE8-0xFF:  grayscale from black to white in 24 steps
                    qreal intensity = qreal(index - 0xE8) / (0xFF - 0xE8);
                    color.setRgbF(intensity, intensity, intensity);
                    break;
                }
                }
                _text_fmt.setBackground(color);
                break;
            }
            default : {
                break;
            }
            }
        }
        break;
    }
    case 49 : {
        _text_fmt.setBackground(_default_fmt.background());
        break;
    }
    case 90 ... 97 : {
        int colorIndex = attribute - 90;
        QColor color;
        switch (colorIndex) {
        case 0 : {
            color = Qt::darkGray;
            break;
        }
        case 1 : {
            color = Qt::red;
            break;
        }
        case 2 : {
            color = Qt::green;
            break;
        }
        case 3 : {
            color = Qt::yellow;
            break;
        }
        case 4 : {
            color = Qt::blue;
            break;
        }
        case 5 : {
            color = Qt::magenta;
            break;
        }
        case 6 : {
            color = Qt::cyan;
            break;
        }
        case 7 : {
            color = Qt::white;
            break;
        }
        default : {
            Q_ASSERT(false);
        }
        }
        color.setRedF(color.redF() * 0.8);
        color.setGreenF(color.greenF() * 0.8);
        color.setBlueF(color.blueF() * 0.8);
        _text_fmt.setForeground(color);
        break;
    }
    case 100 ... 107 : {
        int colorIndex = attribute - 100;
        QColor color;
        switch (colorIndex) {
        case 0 : {
            color = Qt::darkGray;
            break;
        }
        case 1 : {
            color = Qt::red;
            break;
        }
        case 2 : {
            color = Qt::green;
            break;
        }
        case 3 : {
            color = Qt::yellow;
            break;
        }
        case 4 : {
            color = Qt::blue;
            break;
        }
        case 5 : {
            color = Qt::magenta;
            break;
        }
        case 6 : {
            color = Qt::cyan;
            break;
        }
        case 7 : {
            color = Qt::white;
            break;
        }
        default : {
            Q_ASSERT(false);
        }
        }
        color.setRedF(color.redF() * 0.8);
        color.setGreenF(color.greenF() * 0.8);
        color.setBlueF(color.blueF() * 0.8);
        _text_fmt.setBackground(color);
        break;
    }
    default : {
        break;
    }
    }
}



void AnsiEscape::parse_terminal_text( const QString& str , QTextCursor& cur ){

    for ( int i=0; i < str.size(); ++i ) {
        QChar c = str.at(i);

        if ( c.isPrint() ){
            if ( _overwrite )
                cur.deleteChar();
            cur.insertText(QString(c), _text_fmt);


        }else if ( c == '\n' ) {//i && c == '\n' && str.at(i-1) != '\r' )
            qDebug() << "new line before " << cur.blockNumber();
            cur.movePosition(QTextCursor::EndOfLine );
            cur.insertText(QString(c));
            qDebug() << "new line after " << cur.blockNumber();
        }

        else if ( c == '\b' ){
            qDebug() << "before move previous : " << cur.positionInBlock();
            cur.movePosition(QTextCursor::PreviousCharacter );
            qDebug() << "after move previous : " << cur.positionInBlock();
            //cur.deletePreviousChar();
        }
        else if ( c == '\r' ){
            qDebug() << "return before " << cur.blockNumber();
            cur.movePosition(QTextCursor::StartOfLine );
            qDebug() << "return after " << cur.blockNumber();
        }

        else if ( c == '\u0007' ){
            qDebug() << "BELL-----------------------------------";
        }

    }

    _overwrite = false;
}

void AnsiEscape::format_terminal_line( const QString& line ) {

    const QRegExp csi_seq(R"(\x1B([\=>\?])|\x1B\[([\d:;\<\=\>\?]*)(\w))");
    const QRegExp osc_seq(R"(\x1B\]([\d:;<\=>\?A-Z]+)(.*)(\x07|\x9c|\x5c))");

    QTextCursor cursor = _tedit->textCursor();


    /**auto adjust = []( const QString& str ) {
      if ( str.endsWith("\r\n"))
          return str.left(str.size()-1).replace('\u0007',' ');
      return QString(std::move(str)).replace('\u0007',' ');
    };*/

    int last_ptr = 0, offset= 0 ;
    for ( ; offset < line.size(); ){


        int csi_pos = csi_seq.indexIn(line , offset );
        int osc_pos = osc_seq.indexIn(line,offset);


        if ( osc_pos != -1 && osc_pos < csi_pos ){

            if ( osc_pos - last_ptr > 0 ){
                //qDebug() << "text:" << line.mid( last_ptr, osc_pos - last_ptr);
                //cursor.insertText( adjust ( line.mid( last_ptr, osc_pos - last_ptr) ) , _text_fmt);
                parse_terminal_text( line.mid( last_ptr, osc_pos - last_ptr), cursor );
            }

            QString m1 = osc_seq.cap(1);
            QString m2 = osc_seq.cap(2);
            QString m3 = osc_seq.cap(3);

            //qDebug() << "[" << offset << "] osc_seq:" << ":" << m1 << ":" << m2 << ":" << m3;

            offset = osc_pos + osc_seq.matchedLength();
            last_ptr = offset;

        } else if ( csi_pos !=-1 ) {

            if ( csi_pos - last_ptr > 0 ){
                //qDebug() << "text:" << line.mid( last_ptr, csi_pos - last_ptr);
                //cursor.insertText( adjust( line.mid( last_ptr, csi_pos - last_ptr) ) , _text_fmt);
                parse_terminal_text( line.mid( last_ptr, csi_pos - last_ptr) , cursor );
            }

            QString m1 = csi_seq.cap(1);
            QString m2 = csi_seq.cap(2);
            QString m3 = csi_seq.cap(3);

            //qDebug() << "[" << offset << "] csi_seq:" <<  ":" << m1 << ":" << m2 << ":" << m3;


            if ( m1.isEmpty()  ) {


                 if ( m2.contains(";") ){

                     QListIterator< QString > I(m2.split(";"));
                     while (I.hasNext()) {
                         bool ok = false;
                         int attribute = I.next().toInt(&ok);
                         if ( !ok)
                             attribute = 0;
                         parse_color_sequence(attribute, I);
                     }

                 } else {

                    if ( m3=="K" ){
                        // Erase in Line
                        if ( m2.isEmpty() || m2.toInt()==0 ){
                            cursor.movePosition(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
                            cursor.removeSelectedText();
                        }else if ( m2.toInt()==1 ){
                            cursor.movePosition(QTextCursor::StartOfBlock,QTextCursor::KeepAnchor);
                            cursor.removeSelectedText();
                        }else if ( m2.toInt()==2 ){
                            cursor.movePosition(QTextCursor::StartOfBlock);
                            cursor.movePosition(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
                            cursor.removeSelectedText();
                        }

                    }else  if ( m3=="J" ){

                        if ( m2.isEmpty() ){
                            // clear from cursor to end of screen
                        }else if ( m2 =="1" ){
                            // clear from cursor to begin of screen

                        }else if ( m2=="2" ) {
                            // clear screen
                            _tedit->document()->clear();

                        }else if ( m2=="3" ) {
                            // clear history
                        }


                    }else if ( m3== "C" ) {
                        // cursor forward
                        cursor.movePosition(QTextCursor::NextCharacter);

                    }else if ( m3== "D" ) {
                        // cursor backward
                        cursor.movePosition(QTextCursor::PreviousCharacter);

                    }else if ( m3=="P"){
                        // Delete chars
                        for ( int i=m2.toInt(); i >0; --i)
                            cursor.deleteChar();
                        _overwrite = true;

                    }else {
                        bool ok = false;
                        int attribute = m2.toInt(&ok);
                        if ( ok ){

                            QListIterator< QString > I = QStringList({""});
                            parse_color_sequence( attribute , I );

                        }else if ( m3=='m' ){

                            _text_fmt = _default_fmt;

                        }
                    }
                 }
            }

            offset = csi_pos + csi_seq.matchedLength();
            last_ptr = offset;

        } else {
            offset +=1;

        }

    }

    if ( offset - last_ptr > 0 ){
        //qDebug() << "text:" << line.mid( last_ptr, offset - last_ptr);
        //cursor.insertText( adjust( line.mid( last_ptr, offset - last_ptr) ) , _text_fmt);
        parse_terminal_text( line.mid( last_ptr, offset - last_ptr) , cursor );
    }

    _tedit->setTextCursor(cursor);

}








void AnsiEscape::parse_text( const QString& str , QTextCursor& cur ){

    QString out;
    out.resize(str.size());

    QChar* ptr = out.data();
    for ( int i=0; i < str.size(); ++i  ) {
        QChar c = str.at(i);
        if ( c.isPrint() || c == '\n' )
            *ptr++ = str.at(i);
    }
    out.resize( ptr - out.data() );
    cur.insertText( out , _text_fmt );
}

void AnsiEscape::format_line( const QString& line ) {

    const QRegExp csi_seq(R"(\x1B([\=>\?])|\x1B\[([\d:;\<\=\>\?]*)(\w))");
    const QRegExp osc_seq(R"(\x1B\]([\d:;<\=>\?A-Z]+)(.*)(\x07|\x9c|\x5c))");

    QTextCursor cursor = _tedit->textCursor();


    /**auto adjust = []( const QString& str ) {
      if ( str.endsWith("\r\n"))
          return str.left(str.size()-1).replace('\u0007',' ');
      return QString(std::move(str)).replace('\u0007',' ');
    };*/

    int last_ptr = 0, offset= 0 ;
    for ( ; offset < line.size(); ){


        int csi_pos = csi_seq.indexIn(line , offset );
        int osc_pos = osc_seq.indexIn(line,offset);


        if ( osc_pos != -1 && osc_pos < csi_pos ){

            if ( osc_pos - last_ptr > 0 ){
                //qDebug() << "text:" << line.mid( last_ptr, osc_pos - last_ptr);
                //cursor.insertText( adjust ( line.mid( last_ptr, osc_pos - last_ptr) ) , _text_fmt);
                parse_text( line.mid( last_ptr, osc_pos - last_ptr), cursor );
            }

            QString m1 = osc_seq.cap(1);
            QString m2 = osc_seq.cap(2);
            QString m3 = osc_seq.cap(3);

            //qDebug() << "[" << offset << "] osc_seq:" << ":" << m1 << ":" << m2 << ":" << m3;

            offset = osc_pos + osc_seq.matchedLength();
            last_ptr = offset;

        } else if ( csi_pos !=-1 ) {

            if ( csi_pos - last_ptr > 0 ){
                //qDebug() << "text:" << line.mid( last_ptr, csi_pos - last_ptr);
                //cursor.insertText( adjust( line.mid( last_ptr, csi_pos - last_ptr) ) , _text_fmt);
                parse_text( line.mid( last_ptr, csi_pos - last_ptr) , cursor );
            }

            QString m1 = csi_seq.cap(1);
            QString m2 = csi_seq.cap(2);
            QString m3 = csi_seq.cap(3);

            //qDebug() << "[" << offset << "] csi_seq:" <<  ":" << m1 << ":" << m2 << ":" << m3;


            if ( m1.isEmpty()  ) {


                 if ( m2.contains(";") ){

                     QListIterator< QString > I(m2.split(";"));
                     while (I.hasNext()) {
                         bool ok = false;
                         int attribute = I.next().toInt(&ok);
                         if ( !ok)
                             attribute = 0;
                         parse_color_sequence(attribute, I);
                     }

                 } else {

                    bool ok = false;
                    int attribute = m2.toInt(&ok);
                    if ( ok ){

                        QListIterator< QString > I = QStringList({""});
                        parse_color_sequence( attribute , I );

                    }else if ( m3=='m' ){

                        _text_fmt = _default_fmt;

                    }

                 }
            }

            offset = csi_pos + csi_seq.matchedLength();
            last_ptr = offset;

        } else {
            offset +=1;

        }

    }

    if ( offset - last_ptr > 0 ){
        //qDebug() << "text:" << line.mid( last_ptr, offset - last_ptr);
        //cursor.insertText( adjust( line.mid( last_ptr, offset - last_ptr) ) , _text_fmt);
        parse_text( line.mid( last_ptr, offset - last_ptr) , cursor );
    }

    _tedit->setTextCursor(cursor);

}





