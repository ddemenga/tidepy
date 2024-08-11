#ifndef PYTHON_SETTINGS_WIDGET_H
#define PYTHON_SETTINGS_WIDGET_H

#include <QWidget>
#include <QMap>
#include <QSettings>




namespace Ui {
class PythonSettings;
}


namespace widgets {


class PythonSettings : public QWidget
{
    Q_OBJECT

public:
    explicit PythonSettings(QWidget *parent = nullptr);
    ~PythonSettings();

    void set_debugger_dir( const QString& );

    bool is_interpreter_ok();

    const QString& cmd() const ;
    QStringList args() const;
    QStringList path() const ;
    QMap<QString,QString> environ() const ;

    bool add_project_dir_to_path() const ;
    void set_project_dir( const QString& );

    bool save_before_run() const;

    void load_settings(QSettings&);
    void save_settings(QSettings&);

private slots:

    void on_btn_close_clicked();


    void on_actionAdd_triggered();

    void on_actionRemove_triggered();

    void on_interpreter_activated(int index);

    void on_actionUp_triggered();

    void on_actionDown_triggered();

protected:

    struct Interpreter {
        QString cmd,label;
        QStringList path, environ;
    };

    void contextMenuEvent(QContextMenuEvent *e) override;

private:

    void change_interpreter(int index);
    void save_interpreter_state();

private:

    Ui::PythonSettings *ui;

    QMap< int , Interpreter* > _inters;


    bool _dirty = false;

    int _cur_interpreter;

    QString _cur_project_dir, _debugger_dir;


};


} //widgets


#endif // PYTHON_SETTINGS_WIDGET_H
