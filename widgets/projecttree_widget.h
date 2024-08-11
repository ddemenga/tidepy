#ifndef PROJECTTREE_WIDGET_H
#define PROJECTTREE_WIDGET_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QAction>
#include <QIcon>


#include "widgets/treectrl.h"



namespace widgets {




class ProjectTree : public TreeCtrl
{
    Q_OBJECT

public:

    explicit ProjectTree( QWidget *parent = nullptr);
    virtual ~ProjectTree();

    bool is_empty() const;

    bool add_project( const QString& path );

    QStringList cur_projects() const ;
    QString cur_dir_project() const;
    QString cur_dir() const ;
    bool is_project_dir( ) const;

    void add_path(const QString& path);
    QString current_path();
    bool del_path();
    void del_cur_project();

    void toggle_current();
    void expand_current();
    void collapse_current();



signals:

    void project_dir_changed( const QString& path );
    void file_changed( const QString& path );
    void dir_changed( const QString& path );

    void item_renamed( const QString& old_path, const QString& new_path );


protected:

    void contextMenuEvent(QContextMenuEvent *event) override;
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override;

private slots:

    void __on_double_clicked__( const QModelIndex& );
    void __on_clicked__( const QModelIndex& );

private:

    void set_bold(QStandardItem* , bool value);
    void set_action_for_item( const QModelIndex & );

    QModelIndex index_cur_dir() const;
    QModelIndex index_cur_project() const ;

    bool is_dir( const QString& ) const;
    bool is_project_dir( const QModelIndex& ) const;
    bool is_package( const QString& ) const;
    bool is_relative_to( const QString& path , const QString& dir ) const;
    bool is_first_level_child_of( const QString& path , const QModelIndex& dir ) const;

    /**
     * @brief update_path_dir_files
     *      After a directory has been renamed all the subtree node's path value [ index.data(Qt::UserRole) ] must be updated.
     */
    void update_path_dir_files( QModelIndex& );
    void rename( QModelIndex &renamed_index );

    //QModelIndex find( const QModelIndex& , const QString& path );
    //QModelIndex find( const QModelIndex& index , const QString& path );

private:


    QAction _separator;

    QMenu* _context_menu;

    QString _cur_dir,_cur_project;

    QIcon _icon_file,_icon_package, _icon_dir;

};



} // widgets


#endif // PROJECTTREE_WIDGET_H
