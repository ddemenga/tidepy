#ifndef TESEARCHRESULT_H
#define TESEARCHRESULT_H

#include <QPlainTextEdit>



namespace widgets {


class TESearchResult : public QPlainTextEdit
{
    Q_OBJECT


public:
    explicit TESearchResult(QWidget *parent = nullptr);


    void replace_all(const QString& , const QString&, bool sensitivity);

signals:

    void file_selected( const QString& , int line );
    void file_changed( const QString& );

protected:

    void mouseMoveEvent(QMouseEvent*e) override;

    void mouseDoubleClickEvent(QMouseEvent*e) override;


};



} // widgets



#endif // TESEARCHRESULT_H
