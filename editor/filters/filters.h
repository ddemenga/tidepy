#ifndef FILTERS_H
#define FILTERS_H

#include <QTextStream>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QString>


namespace text_editor {
namespace filters {


class Filter {

public:

    virtual bool run( QTextCursor&  ) const = 0;
    virtual ~Filter();

    void set_tab_size( int );
    inline int tab_size() const { return _tab_size; }

protected:

    QString _tab_space = QString(4,QChar(' '));

private:

    int _tab_size = 4;

};

struct LoopOverSequence : Filter {

    bool run( QTextCursor&  ) const override;

};

struct LoopOverMap : Filter {

    bool run( QTextCursor&  ) const override;

};



struct CaseFilter : Filter {

    bool run( QTextCursor&  ) const  override;

};




struct ClassFilter : Filter {


    bool run( QTextCursor&  ) const override;

};


struct DefProperty : Filter {

    bool run( QTextCursor&  ) const override;

};

struct MemberFunctionFilter : Filter {

    bool run( QTextCursor&  ) const override;

};


struct FunctionFilter : Filter {

    bool run( QTextCursor&  ) const override;

};



}
}

#endif // FILTERS_H
