#ifndef SEARCH_WIDGET_H
#define SEARCH_WIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include <QDir>


namespace Ui {
class SearchWidget;
}


namespace widgets  {


class SearchWidget : public QWidget
{
    Q_OBJECT

public:

    explicit SearchWidget(QWidget *parent = nullptr);
    ~SearchWidget();


signals:

    void file_selected( const QString& , int line );
    void file_changed( const QString& );

public slots:

    void set_search_dir( const QDir& );
    void set_current_file( const QFileInfo& path);


private slots:
    void on_actionFind_triggered();

    void on_actionReplace_triggered();

private:
    Ui::SearchWidget *ui;

    QButtonGroup _buttons;

    QDir _search_dir;
    QString _current_file;

    QString _regexp_pattern;
};


}// widgets



#endif // SEARCH_WIDGET_H
