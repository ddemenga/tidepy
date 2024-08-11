#ifndef VARS_WIDGET_H
#define VARS_WIDGET_H

#include <QWidget>
#include <QStandardItemModel>

#include <py_editor_tools/debugger_objects.h>
#include "widgets/vars_delegate.h"

namespace Ui {
class VarsWidget;
}


namespace widgets {



class VarsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VarsWidget(QWidget *parent = nullptr);
    ~VarsWidget();

signals:
    void var_added( const debugger::VarInfo& );
    void var_deleted( int id  );
    void reload_vars();

public slots:
    void set_var( const debugger::VarInfo&  );
    void set_vars( const debugger::QVarInfoList&  );
    void var_name_changed( int id  );

private slots:

    void on_actionAdd_Watch_triggered();
    void on_actionRemove_Watch_triggered();
    void on_actionReload_triggered();

private:

    template < typename Func >
    void foreach_row( Func func ){
        for ( int irow = 0; irow < _model->rowCount(); irow++){
            func ( irow , _model->item(irow)->data(Qt::UserRole).toInt() );
        }
    }

private:
    Ui::VarsWidget *ui;

    QStandardItemModel* _model;
    VarsDelegate* _delegate;

    static int _ids;

};



}//widgets



#endif // VARS_WIDGET_H
