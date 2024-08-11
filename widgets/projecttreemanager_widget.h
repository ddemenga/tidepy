#ifndef PROJECTTREEMANAGER_WIDGET_H
#define PROJECTTREEMANAGER_WIDGET_H

#include <QWidget>
#include <QSettings>


namespace Ui {
class ProjectTreeManager;
}


namespace widgets {

class ProjectTreeManager : public QWidget
{
    Q_OBJECT

public:

    enum Type { SCRIPT, MODULE , PACKAGE, DIR };


    explicit ProjectTreeManager(QWidget *parent = nullptr);
    ~ProjectTreeManager();

    void load_settings(QSettings&);
    void save_settings(QSettings&);


    bool is_empty() const;

    void set_new_file(QAction*) ;
    void set_new_dir(QAction*) ;
    void set_add_project(QAction*) ;
    void set_del_project(QAction*) ;

    QStringList cur_projects() const;
    QString cur_project() const;
    QString cur_dir() const;


    bool is_relative( const QString& path ) const ;


public slots:

    bool add_project( const QString& path );

    void set_enable_project_tools( bool );

signals:



    void new_file_added( const QString&);
    void new_dir_added( const QString&);

    void file_changed( const QString& path );
    void dir_changed( const QString& path );
    void project_dir_changed( const QString& path );

    void item_renamed( const QString& old_path, const QString& new_path );

    void file_deleted( const QString& );
    void dir_deleted( const QString& );

private slots:

    void __on_actionNewFile_triggered__();
    void __on_actionNewDir_triggered__();
    void on_actionDelete_triggered();


private:

    QString normalize_path( const QString& path , Type type );


private:
    Ui::ProjectTreeManager *ui;

    QAction* _new_dir;
    QAction* _new_file;
};




} // widgets


#endif // PROJECTTREEMANAGER_WIDGET_H
