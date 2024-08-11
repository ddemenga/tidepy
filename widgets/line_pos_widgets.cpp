#include "line_pos_widgets.h"
#include "ui_line_pos_widgets.h"


namespace widgets {




LineColPosWidget::LineColPosWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LineColPosWidget)
{
    ui->setupUi(this);
}

LineColPosWidget::~LineColPosWidget()
{
    delete ui;
}


void LineColPosWidget::set_position( int line , int col, int pos ) {

    ui->lbl_line->setText(QString("Line: %1").arg(line));
    ui->lbl_col->setText(QString("Col: %1").arg(col));
    ui->lbl_pos->setText(QString("Pos: %1").arg(pos));

}



} // widgets
