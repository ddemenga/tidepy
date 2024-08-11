#ifndef TEXTEDITBASE_H
#define TEXTEDITBASE_H


#include <QPlainTextEdit>
#include <QKeySequence>
#include <QShortcut>
#include <QMap>
#include <QFileInfo>
#include <QList>
#include <QCompleter>




#include "syntaxhighlighter.h"
#include "filters/filters.h"




namespace text_editor {


struct TextEditorHighlighter {

    SyntaxHighlighter* sh;
    ISyntaxHighighter* parser;

};




class DocAction;

class TextEditBase : public QPlainTextEdit
{
    Q_OBJECT



public:



    explicit TextEditBase(QWidget *parent = 0);
    virtual ~TextEditBase();


    void setCompleter(QCompleter *c);
    QCompleter *completer() const;

    int id() const;
    void set_id(int );

    void set_filename(const QFileInfo & );
    const QFileInfo filename() const;

    bool is_active() const ;
    void set_active( bool );

    bool is_dirty() const;


    void clear_text();
    void load_text( const QString& );
    void load_file( const QString& );
    void reload_content();

    bool open();
    bool open( const QString& filepath );

    bool save_as();
    bool save( );

    unsigned lineNumberAreaWidth();

    inline int tab_size() const { return _tab_size; }
    void set_tab_size( int );

    void add_snippet( QAction* a, text_editor::filters::Filter* s, const QKeySequence& kseq );



signals:

    void cursor_pos_changed( int line ,int col , int pos );
    void filename_changed( int id , const QString& );
    void state_changed(int id);




protected:

    void mouseReleaseEvent( QMouseEvent* );
    void keyPressEvent(QKeyEvent *) override;
    void focusInEvent(QFocusEvent *) override;
    void wheelEvent(QWheelEvent* ) override;

    bool save( const QString& filepath );

    QString textUnderCursor() const;


    struct Snippet {

        Snippet(QWidget* parent , text_editor::filters::Filter* f ,const QKeySequence& ks ) : filter(f), kseq(ks,parent) {}

        ~Snippet() {

            kseq.parent()->disconnect( con );
            delete filter;
        }

        text_editor::filters::Filter* filter;
        QShortcut kseq;
        QMetaObject::Connection con;

    };



private slots:

    void insert_completion(const QString &completion);


private:


    template< typename Filter >
    void do_filter( const Filter& filter ) {

        static_assert( std::is_base_of_v< text_editor::filters::Filter, Filter > , "Must be a text_editor::filters::Filter subclass" );

        QTextCursor tc = textCursor();
        tc.beginEditBlock();
        if ( filter.run( tc ) )
            setTextCursor(tc);
        tc.endEditBlock();
    }



protected :

    TextEditorHighlighter _highlighter;

    int _tab_size;

    QMap< QAction* , Snippet* > _snippets;



private:

    int _id = -1;

    bool _active, _dirty;

    QString _base_dir;
    QFileInfo _filename;

    QCompleter *_completer = nullptr ;

    QMap< QKeySequence, DocAction* > _actions;

    QList<QShortcut*> _shortcuts;

};



}// text_editor




#endif // TEXTEDITBASE_H
