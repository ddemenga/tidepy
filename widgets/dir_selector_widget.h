#ifndef DIR_SELECTOR_WIDGET_H
#define DIR_SELECTOR_WIDGET_H

#include <QWidget>

namespace Ui {
class DirSelector;
}

class DirSelector : public QWidget
{
    Q_OBJECT

public:
    explicit DirSelector(QWidget *parent = nullptr);
    ~DirSelector();

    void set_directory( const QString& );
    QString get_directory();

signals :

    void dir_changed( const QString& );


private slots:

    void on_actionChange_Dir_triggered();

private:
    Ui::DirSelector *ui;
};

#endif // DIR_SELECTOR_WIDGET_H
