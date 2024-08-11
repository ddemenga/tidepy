#ifndef DOC_ACTION_SHIFTTAB_H
#define DOC_ACTION_SHIFTTAB_H

#include <QObject>
#include <QPlainTextEdit>

namespace text_editor {


class  DocAction {

public:

    virtual bool can_handle_keyseq ( QPlainTextEdit* editor ,const QKeySequence&  ) = 0;
    virtual bool run( QPlainTextEdit* , const QKeySequence &) = 0 ;

    void set_tab_size( int );
    inline  int tab_size() const { return _tab_size; }
    inline const QString& tab_spaces() const { return _tab_spaces; }

private:

    int _tab_size = 4;
    QString _tab_spaces = QString( 4 , QChar(' '));

};

class DocShiftTabAction : public QObject , public DocAction
{
    Q_OBJECT

public:

    explicit DocShiftTabAction(QObject *parent = nullptr);

    bool run( QPlainTextEdit* , const QKeySequence& kseq ) override;
    bool can_handle_keyseq (QPlainTextEdit *editor, const QKeySequence&  ) override;


private:

    void next_block(  QTextCursor& tc) const;
    void sel_next_block(  QTextCursor& tc) const;

private:

    const QKeySequence _tab = QKeySequence(Qt::Key_Tab);
    const QKeySequence _shift_tab = QKeySequence( Qt::SHIFT | Qt::Key_Backtab );

};


class DocTabAction : public QObject , public DocAction
{
    Q_OBJECT

public:

    explicit DocTabAction(QObject *parent = nullptr);

    bool run( QPlainTextEdit* , const QKeySequence& kseq ) override;
    bool can_handle_keyseq (QPlainTextEdit *editor, const QKeySequence&  ) override;



};


class DocReturnAction : public QObject , public DocAction
{
    Q_OBJECT

public:

    explicit DocReturnAction(QObject *parent = nullptr);


    bool run( QPlainTextEdit* , const QKeySequence& kseq ) override;
    bool can_handle_keyseq (QPlainTextEdit *editor, const QKeySequence&  ) override;




};



class DocBackspaceAction : public QObject , public DocAction
{
    Q_OBJECT

public:

    explicit DocBackspaceAction(QObject *parent = nullptr);


    bool run( QPlainTextEdit* , const QKeySequence& kseq ) override;
    bool can_handle_keyseq (QPlainTextEdit *editor, const QKeySequence&  ) override;



};


} // text_editor
#endif // DOC_ACTION_SHIFTTAB_H
