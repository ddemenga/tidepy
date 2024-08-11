#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QFileDialog>



#include <filesystem>
namespace fs = std::filesystem;



#include "widgets/projecttree_widget.h"



namespace {

struct Crawler {

    int count;

    inline bool is_package( const QString& path ){
        return QFileInfo(QDir(path).filePath("__init__.py")).exists();
    }




    bool load( const fs::path& dir , QStandardItem *parent ){

        bool ret = true;
        for (auto const& dir_entry : std::filesystem::directory_iterator{dir}){

            auto path = dir_entry.path();


            try {
                if ( fs::is_directory( path ) ) {
                    if ( path.stem() == "__pycache__")
                        continue;

                    QStandardItem *item;
                    if ( fs::is_regular_file( path / "__init__.py" ) ){

                        QIcon icon;
                        icon.addFile(":/app_data/icons/folder-documents.svg", QSize(), QIcon::Normal, QIcon::Off);

                        item = new QStandardItem( icon, QString::fromStdString(path.filename()));
                        item->setData(QString::fromStdString(path) ,Qt::UserRole );
                        //item->setEditable(false);
                        parent->appendRow({item});

                    }else{

                        QIcon icon;
                        icon.addFile(":/app_data/icons/folder.svg", QSize(), QIcon::Normal, QIcon::Off);
                        item = new QStandardItem( icon, QString::fromStdString(path.filename()));
                        item->setData(QString::fromStdString(path) ,Qt::UserRole );
                        //item->setEditable(false);
                        parent->appendRow({item});
                    }
                    count++;

                    ret = load( path , item );

                }else if ( ( fs::is_regular_file(path) && path.extension() == ".py" ) ){

                    QIcon icon;
                    icon.addFile(":/app_data/icons/text-x-python.svg", QSize(), QIcon::Normal, QIcon::Off);

                    QStandardItem *item = new QStandardItem( icon, QString::fromStdString(path.filename()));
                    item->setData(QString::fromStdString(path) ,Qt::UserRole );
                    if ( path.stem() == "__init__" )
                        item->setEditable(false);
                    parent->appendRow({item});

                    count++;
                }

            }catch(...){
                return false;
            }
        }

        return ret;
    }
};

}

/** == ProjectTree ===================================================== **/


namespace widgets {



ProjectTree::ProjectTree(QWidget *parent)
    : TreeCtrl{parent}
{

    _model = new QStandardItemModel;
    setModel( _model );
    //setExpandsOnDoubleClick(false);

    //header()->stretchLastSection();
    //header()->setDefaultAlignment(Qt::AlignLeft);
    set_columns({});
    //header()->hide();
    //setAlternatingRowColors(true);

    connect( this, &ProjectTree::doubleClicked , this , &ProjectTree::__on_double_clicked__ );
    connect( this , &QTreeView::clicked , this , &ProjectTree::__on_clicked__  );

    setEditTriggers( QAbstractItemView::SelectedClicked );


    _icon_file.addFile(":/app_data/icons/text-x-python.svg", QSize(), QIcon::Normal, QIcon::Off);
    _icon_package.addFile(":/app_data/icons/folder-documents.svg", QSize(), QIcon::Normal, QIcon::Off);
    _icon_dir.addFile(":/app_data/icons/folder.svg", QSize(), QIcon::Normal, QIcon::Off);

    setSortingEnabled(true);

}


ProjectTree::~ProjectTree() {

}

bool ProjectTree::is_empty() const {
    return _model->invisibleRootItem()->rowCount() == 0;
}
bool ProjectTree::is_package( const QString& path ) const {
    return QFileInfo(path).isDir() && QFileInfo( QDir(path).absoluteFilePath("__init__.py")).isFile() ;
}

bool ProjectTree::is_dir( const QString& path ) const {
    return QFileInfo(path).isDir();
}
bool ProjectTree::is_project_dir(  ) const{
    QModelIndex index = currentIndex();
    return index.isValid()  && _model->itemFromIndex( index.parent() ) == nullptr;
}
bool ProjectTree::is_project_dir( const QModelIndex& index ) const{
    return _model->itemFromIndex(index.parent() ) == nullptr;
}

bool ProjectTree::is_relative_to( const QString& path , const QString& dir ) const{
    return path.startsWith(dir);
}

bool ProjectTree::is_first_level_child_of( const QString& path , const QModelIndex& dir ) const{
    QString dir_path = dir.data(Qt::UserRole).toString();

    return QFileInfo(path).dir().absolutePath() == dir_path;

}

QModelIndex ProjectTree::index_cur_dir() const {

    QModelIndex index = currentIndex();

    for ( ; index.isValid() && !QFileInfo(index.data(Qt::UserRole).toString()).isDir();  index = index.parent());
    if ( index.isValid() && QFileInfo(index.data(Qt::UserRole).toString()).isDir() )
        return index;

    return QModelIndex();

}

QModelIndex ProjectTree::index_cur_project() const {
    QModelIndex index = currentIndex();

    //qDebug() << index.data(Qt::UserRole).toString();
    for ( ; index.isValid()  & !is_project_dir(index) ;  index = index.parent());
        //qDebug() << index.data(Qt::UserRole).toString();

    if ( index.isValid() &&  QFileInfo (index.data(Qt::UserRole).toString()).isDir() )
        return index;
    return QModelIndex();
}

QString ProjectTree::cur_dir() const {

    QModelIndex index = index_cur_dir();
    if ( index.isValid() )
        return index.data(Qt::UserRole).toString();
    return "";
}

QString ProjectTree::cur_dir_project() const {

    QModelIndex index = index_cur_project();
    if ( index.isValid() )
        return index.data(Qt::UserRole).toString();
    return "";
}



QString ProjectTree::current_path() {

    QModelIndex index= currentIndex();

    if ( index.isValid() )
        return index.data(Qt::UserRole).toString();

    return "";
}



bool ProjectTree::add_project( const QString& path ){

    if ( !fs::is_directory( path.toStdString() ) )
         return false;

    auto* root = _model->invisibleRootItem();

    QIcon icon;
    icon.addFile(":/app_data/icons/gnome-mime-application-vnd.openxmlformats-officedocument.presentationml.presentation.svg", QSize(), QIcon::Normal, QIcon::Off);
    QStandardItem * item_path = new QStandardItem( icon, path );
    item_path->setData( path  ,Qt::UserRole );
    item_path->setEditable(false);
    set_bold( item_path , true );
    root->appendRow({item_path});

    Crawler c{};
    c.count= 0;
    if ( c.load( path.toStdString() , item_path   ) ){

        setCurrentIndex( _model->indexFromItem( item_path ) );

        sortByColumn(0,Qt::AscendingOrder);

        return true;
    }

    return false;
}





void ProjectTree::add_path(const QString&  path ) {


    QModelIndex dir_index = index_cur_dir();


    if ( !dir_index.isValid() ) {
        return;
    }

    if ( ! is_first_level_child_of( path , dir_index ) )
        return;


    //QString dir = dir_index.data(Qt::UserRole).toString();
    QStandardItem* parent_item = _model->itemFromIndex(dir_index);


    QStandardItem *item;
    if ( is_dir(path) ){

        auto found = TreeCtrl::find( index_cur_project() , [ path ]( const QModelIndex& i ){
            if ( i.data(Qt::UserRole).toString() == path ){
                return true;
            }
            return false;

        } );


        if (  found.isValid() ){
            QMessageBox::warning(this, "Add directory", "Already exists");
            return;
        }

        item =  new QStandardItem( QFileInfo(path).completeBaseName() ) ;
        item->setIcon( _icon_dir );
        item->setData( path  ,Qt::UserRole );
        parent_item->appendRow({item});

    }else{

        QStandardItem *item = new QStandardItem( QString("%1.py").arg(QFileInfo(path).completeBaseName()) );
        item->setIcon( _icon_file );
        item->setData( path  ,Qt::UserRole );
        parent_item->appendRow({item});

        QString parent_path = parent_item->data(Qt::UserRole).toString();
        is_package(parent_path) ? parent_item->setIcon( _icon_package ) : parent_item->setIcon( _icon_dir );

    }


    sortByColumn(0,Qt::AscendingOrder);


}


void ProjectTree::del_cur_project(){

    QModelIndex index = index_cur_project();
    if ( index.isValid() ){
        _model->removeRow( index.row() , index.parent() );
    }

}

bool ProjectTree::del_path() {


    QModelIndex index = currentIndex();

    if ( index.isValid() ){

        QString path = index.parent().data(Qt::UserRole).toString();


        QModelIndex parent = index.parent();
        auto * item = _model->itemFromIndex(parent);


        if ( item == nullptr  )
            // is project
            return false;

        else{


            if  ( is_dir(path) && !is_package( path  ) ){
                QStandardItem* item = _model->itemFromIndex(index.parent());
                item->setIcon( _icon_dir );
            }
            _model->removeRow( index.row() , index.parent() );

            return true;
        }
        //QFileInfo info( index.data(Qt::UserRole).toString() );

    }

    return true;

}


void ProjectTree::update_path_dir_files( QModelIndex& index ) {


    QString dir_path( index.data(Qt::UserRole).toString());
    for ( int i=0; i < _model->rowCount( index ); i++ ){

        QModelIndex c  = _model->index(i,0,index);
        QFileInfo info(c.data(Qt::UserRole).toString());

        QString old_path = c.data(Qt::UserRole).toString();
        QString new_path = QString("%1/%2").arg( dir_path ).arg( info.fileName());
        _model->setData( c, new_path  , Qt::UserRole );

        emit item_renamed( old_path , new_path );
        if ( QFileInfo(new_path).isDir()  )
            update_path_dir_files(c);

    }


}


void ProjectTree::rename( QModelIndex &renamed_index ) {

    if ( renamed_index.isValid() ){
        // Validate value

        QString old_path = renamed_index.data(Qt::UserRole).toString();
        QString new_name = renamed_index.data(Qt::DisplayRole).toString();
        QString new_path = QFileInfo(old_path).dir().absoluteFilePath( new_name );

        QRegExp validate;
        bool is_file = false;
        if ( old_path.endsWith(".py") ){
            is_file = true;
            validate.setPattern(R"(^(\w[\d\w_.\-]+.py)$)");
        }else{
            validate.setPattern(R"(^(\w[\d\w_.\-]+)$)");
        }

        int pos = validate.indexIn( new_name );
        if ( pos == 0 ){
            // the name is validated



            if ( old_path != new_path ){
                QFileInfo info(old_path);

                bool ok = info.dir().rename(old_path,new_path);

                if ( ok ){

                    _model->setData( renamed_index, new_path,Qt::UserRole);
                    update_path_dir_files( renamed_index );

                    emit item_renamed( old_path , new_path );
                }else{

                    // put things back
                    if ( is_file )
                        _model->setData( renamed_index , QString("%1.%2").arg( QFileInfo(old_path).completeBaseName() ).arg( QFileInfo(old_path).suffix() ), Qt::DisplayRole );
                    else
                        _model->setData( renamed_index , QString("%1").arg( QFileInfo(old_path).completeBaseName() ) , Qt::DisplayRole );

                    QMessageBox::critical(this, "Error" , QString("Failed to rename %1").arg(info.absoluteFilePath()));

                }
            }

        } else {
            // not validated

            // put things back
            if ( is_file )
                _model->setData( renamed_index , QString("%1.%2").arg( QFileInfo(old_path).completeBaseName() ).arg( QFileInfo(old_path).suffix() ), Qt::DisplayRole );
            else
                _model->setData( renamed_index , QString("%1").arg( QFileInfo(old_path).completeBaseName() ) , Qt::DisplayRole );
        }


    }
}

void ProjectTree::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) {
    QAbstractItemView::closeEditor(editor, hint);

    QModelIndex renamed_index = currentIndex();
    rename( renamed_index );

}

void ProjectTree::contextMenuEvent(QContextMenuEvent *e) {

    auto list_actions = actions();

    if (  !list_actions.size() )
        return;

    QMenu menu;
    menu.addActions( list_actions );
    menu.popup( mapToGlobal( e->pos() ) );

    e->accept();
}







QStringList ProjectTree::cur_projects() const {

    QStringList  ret;

    auto* root = _model->invisibleRootItem();
    for ( int i=0; i < root->rowCount(); ++i ){
        auto * item = root->child(i);
        if ( item )
            ret.append(item->data(Qt::UserRole).toString());
    }

    return ret;

}




void ProjectTree::set_bold( QStandardItem* item , bool value  ) {

    QFont font = item->font();
    font.setBold(value);
    item->setFont( font );
}




void ProjectTree::__on_double_clicked__( const QModelIndex& index ) {

    if ( !index.isValid() )
        return;

    if ( is_dir( index.data(Qt::UserRole).toString()) ){

        QModelIndex parent = index.parent();
        if ( _model->itemFromIndex(parent) == nullptr )
            emit project_dir_changed( index.data(Qt::UserRole).toString() );
         else{
            emit project_dir_changed( index_cur_project().data(Qt::UserRole).toString());
            emit dir_changed( index.data(Qt::UserRole).toString() );
        }
    }else{

        emit project_dir_changed( index_cur_project().data(Qt::UserRole).toString());
        emit dir_changed( index.parent().data(Qt::UserRole).toString());
        emit file_changed( index.data(Qt::UserRole).toString());

    }



}


void ProjectTree::__on_clicked__( const QModelIndex& index ) {

    if ( !index.isValid() )
        return;

    if ( is_dir( index.data(Qt::UserRole).toString()) ){

        QModelIndex parent = index.parent();
        if ( _model->itemFromIndex(parent) == nullptr )
            emit project_dir_changed( index.data(Qt::UserRole).toString() );
        else{
           emit project_dir_changed( index_cur_project().data(Qt::UserRole).toString());
           emit dir_changed( index.data(Qt::UserRole).toString() );
        }
    }else{

        emit project_dir_changed( index_cur_project().data(Qt::UserRole).toString());
        emit dir_changed( index.parent().data(Qt::UserRole).toString());


    }

}

void ProjectTree::toggle_current() {
    QModelIndex index = currentIndex();

    if ( !index.isValid() )
        return;

    if ( isExpanded(index) )
        collapse(index);
    else
        expand(index);
}

void ProjectTree::expand_current() {
    QModelIndex index = currentIndex();
    if( index.isValid() && is_dir( index.data(Qt::UserRole).toString() ))
        expand(index);
}
void ProjectTree::collapse_current(){
    QModelIndex index = currentIndex();
    if( index.isValid() && is_dir( index.data(Qt::UserRole).toString() ))
        collapse(index);
}



/**
QModelIndex ProjectTree::find( const QString& path ){

    auto* root = _model->invisibleRootItem();
    for ( int i=0; i < root->rowCount(); ++i ){
        auto * item = root->child(i);
        if ( item ){
            QModelIndex index = find ( item->index(), path );
            if ( index.isValid() )
                return index;
        }
    }

    return QModelIndex();
}

QModelIndex ProjectTree::find( const QModelIndex& index , const QString& path ){

    if ( index.data(Qt::UserRole).toString() == path )
        return index;

    for ( int i=0; i < _model->rowCount( index ); i++ ){

        QModelIndex c = find( _model->index(i,0,index) , path );
        if ( c.isValid() )
            return c;
    }

    return QModelIndex();
}
**/

} // widgets
