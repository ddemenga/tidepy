#ifndef QSCOREEDITOR_H
#define QSCOREEDITOR_H


#include <memory>


#include <QWidget>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QRegExp>
#include <QColor>
#include <QFont>
#include <QResizeEvent>
#include <QRect>
#include <QBrush>
#include <QCompleter>
#include <QIcon>
#include <QMap>
#include <QAction>
#include <QShortcut>
#include <type_traits>
//#include <QDebug>

#include "decorated_editor_base.h"
#include "extraselections.h"




#include "py_editor_tools/debugger_objects.h"
namespace dbg = debugger;




namespace text_editor {



class TextEditor;
class Patch;


class QBreakpoint : public QTextBlockUserData {

public:

    enum Type { NONE , ENABLED , DISABLED };

    QBreakpoint(  );

    virtual ~QBreakpoint();



    inline bool is_enabled() const {
        return _state == ENABLED;
    }

    inline bool is_disabled() const {
        return _state == DISABLED;
    }

    inline bool is_none() const{
        return _state == NONE;
    }


    inline void enable() {
        _state = ENABLED;
    }

    inline void disable() {
        _state = DISABLED;
    }

    inline void remove(){
        _state = NONE;
    }


    inline void toggle() {

        if ( is_enabled() )
            _state = DISABLED;
        else if ( is_disabled() )
            _state = NONE;
        else {
            _state = ENABLED;
        }


    }


private:


    Type _state;

};



class LineNumbersArea : public QWidget{
    Q_OBJECT

public:

    LineNumbersArea(TextEditor *parent = nullptr);

    void set_bg_color( const QColor& );
    const QColor bg_color();

    unsigned getWidth();

signals:

    void line_selected ( int );

protected:

    void paintEvent( QPaintEvent* ) override;
    void mouseReleaseEvent( QMouseEvent* ) override;
    //void mouseDoubleClickEvent(QMouseEvent *) override;

public slots:

    void updateWidth();
    void updateContents(const QRect &rect, int dy);

private:

    QColor _bg_color,_bold,_normal;
    QFont _font;

    TextEditor* _editor;

    QIcon _break_icon;
};



/**
 * @brief The Breakpoint struct

struct Breakpoint {
    int lineno;
    bool enabled;
};
 */



/**
 * @brief The TextEditor class
 */
class TextEditor : public DecoratedEditorBase
{
    Q_OBJECT

    friend class LineNumbersArea;
    friend class QBreakpoint;

public:

    explicit TextEditor(QWidget *parent = nullptr);
    virtual ~TextEditor();

    std::shared_ptr< Patch> patch() ;
    void set_patch ( const std::shared_ptr< Patch>& value );



    void scroll_to( unsigned line, unsigned column  = 0);
    void set_debug_line( unsigned line );

    void clear_extra_selections();
    void do_extra_selections();

    int num_lines() const;
    int line() const;
    int column() const;
    QString text() const;

    bool exit( bool check_dirty = true );
    void set_highlighter(const TextEditorHighlighter &teh );



    void set_breakpoint( int line , bool enabled );
    dbg::QBreakInfoList get_breakpoints() const;

    void set_debugging_mode(bool value);
    inline bool is_debugging() const { return _debugging_mode; }


    void clear_all();


public slots:

    void updateLineNumberAreaWidth();


signals:

    void content_changed( TextEditor* );
    void remove_document( TextEditor* );
    void breaks_changed(const QString& filename , const dbg::QBreakInfoList& );



protected:

    void resizeEvent(QResizeEvent *) override ;

private slots:

    void __on__linenumber_selected__( int  );

private:

    QAbstractItemModel* modelFromFile(QCompleter *completer, const QString& fileName);

private:

    unsigned _line_numbers_space ;
    int _ln;

    LineNumbersArea* _line_numbers_area;

    QCompleter* _completer = nullptr;

    ExtraSelections _extra_selector;


    QList< QBreakpoint* > _list_breaks;




    bool _debugging_mode = false;
};



} // text_editor
#endif // QSCOREEDITOR_H
