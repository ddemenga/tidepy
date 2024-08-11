#ifndef FILESTRUCTURE_H
#define FILESTRUCTURE_H

#include <QWidget>
#include <QTemporaryFile>
#include <QAction>

namespace Ui {
class FileStructure;
}


namespace widgets {



class FileStructure : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int document_id READ document_id )
public:

    static constexpr int no_document = -1;

    explicit FileStructure(QWidget *parent = nullptr);
    ~FileStructure();

    inline void set_script_path(const QString& value )  { _py_script = value; }

    int document_id() const ;
    void clear();

    void update(int id, const QString& filepath );
    QAction* refresh();



signals:

    void position_selected( int line , int pos );

private slots:
    void on_actionToggle_triggered();

private:
    Ui::FileStructure *ui;

    int _document_id;

    QString _py_script;
};


} // widgets



#endif // FILESTRUCTURE_H
