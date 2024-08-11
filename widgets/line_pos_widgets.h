#ifndef LINE_POS_WIDGETS_H
#define LINE_POS_WIDGETS_H

#include <QWidget>

namespace Ui {
class LineColPosWidget;
}


namespace widgets {

class LineColPosWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LineColPosWidget(QWidget *parent = nullptr);
    ~LineColPosWidget();


public slots:

    void set_position( int line , int col , int pos );


private:
    Ui::LineColPosWidget *ui;
};


} //widgets

#endif // LINE_POS_WIDGETS_H
