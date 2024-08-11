//#include <pybind11/embed.h> // everything needed for embedding
//namespace py = pybind11;
//using namespace py::literals;

#include "filestructure.h"
#include "ui_filestructure.h"

#include <fmt/core.h>

#include "rapid_xml/rapidxml_ns.hpp"
#include "rapid_xml/rapidxml_ns_print.hpp"
#include "rapid_xml/rapidxml_ns_utils.hpp"
namespace xml = rapidxml_ns;

#include <QTreeWidget>
#include <QPoint>
#include <QTemporaryFile>
#include <QDebug>
#include <QToolButton>
#include <QHBoxLayout>
#include <QModelIndex>
#include <QProcess>
#include <QDir>




namespace widgets {


namespace {

struct ParseTree {

    enum Type { IMPORT, IMPORT_FROM , VAR, FUNCTION, CLASS , MEMBER_VAR, MEMBER_ATTR, MEMBER_FUNCTION };


    void parse( xml::xml_document<char>& doc , QTreeWidget *tree){

        QTreeWidgetItem * node = tree->invisibleRootItem(); //new QTreeWidgetItem( (QTreeWidgetItem*)nullptr, QStringList( "Structure" ) << "" , 0) ;
        //tree->addTopLevelItem(node);

        for ( auto* n = doc.first_node(); n ; n = n->next_sibling() )
            parse(n , node );

        tree->sortByColumn(0,Qt::AscendingOrder);
    }

    void  parse( xml::xml_node<char>* n , QTreeWidgetItem* parent  ) {

        if ( !n )
            return ;

        int line = std::atoi( n->first_attribute("line")->value() );
        int col = std::atoi(  n->first_attribute("column")->value() );
        std::string id = n->first_attribute("id")->value();
        std::string name = n->name();

        QString obj_type = QString::fromStdString(name);

        QTreeWidgetItem * node;
        if ( obj_type.startsWith( "class" ) ){
            node = new QTreeWidgetItem( parent, QStringList( QString::fromStdString(id)  ) << obj_type  ,1 ) ;
            QIcon icon(":/app_data/icons/extension.svg");//application-x-deb.svg");
            node->setIcon(0,icon);
        }else if (obj_type.startsWith( "import" ) ){
            node = new QTreeWidgetItem( parent, QStringList( QString::fromStdString(id)  ) << "import"  ,1 ) ;
            QIcon icon(":/app_data/icons/document-import.svg");
            node->setIcon(0,icon);
        }else if ( obj_type.startsWith("member_attr") ){
            node = new QTreeWidgetItem( parent, QStringList( QString::fromStdString(id)  ) << "class attribute"  ,1 ) ;
            QIcon icon(":/app_data/icons/tag-new.svg");
            node->setIcon(0,icon);
        }else if ( obj_type.startsWith("member_function")){
            node = new QTreeWidgetItem( parent, QStringList( QString::fromStdString(id)  ) << "class method"  ,1 ) ;
            QIcon icon(":/app_data/icons/member_function.svg");
            node->setIcon(0,icon);
        }else if ( obj_type.startsWith("function")){
            node = new QTreeWidgetItem( parent, QStringList( QString::fromStdString(id)  ) << obj_type  ,1 ) ;
            QIcon icon(":/app_data/icons/document-export.svg");
            node->setIcon(0,icon);
        }else if ( obj_type.startsWith("member_var")){
            node = new QTreeWidgetItem( parent, QStringList( QString::fromStdString(id)  ) << "class global variable"  ,1 ) ;
            QIcon icon(":/app_data/icons/text-x-generic-template.svg");
            node->setIcon(0,icon);

        }else {
            node = new QTreeWidgetItem( parent, QStringList( QString::fromStdString(id)  ) << obj_type  ,1 ) ;
            QIcon icon(":/app_data/icons/application-octet-stream.svg");
            node->setIcon(0,icon);
        }

        node->setData( 1, Qt::UserRole, QPoint( line, col ));
        parent->addChild(node);

        for ( auto* c = n->first_node(); c ; c = c->next_sibling() )
            parse(c , node );
    }
};


}//




FileStructure::FileStructure(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileStructure),_document_id(no_document)
{
    ui->setupUi(this);



    ui->refresh->setDefaultAction( ui->actionRefresh);
    ui->toggle->setDefaultAction( ui->actionToggle );


    ui->tree->setColumnCount(2);
    ui->tree->setHeaderLabels( {"object", "type"} );
    ui->tree->setExpandsOnDoubleClick(false);

    connect( ui->tree , &QTreeWidget::itemDoubleClicked , [&]( QTreeWidgetItem *item, int /*column*/ ){

        if ( item->type() == 1  ){
            QPoint p = item->data(1, Qt::UserRole).toPoint();

            emit position_selected( p.x() , p.y() );
        }
    });


}



FileStructure::~FileStructure()
{
    delete ui;
}
int FileStructure::document_id() const {
    return _document_id;
}

QAction* FileStructure::refresh(){
    return ui->actionRefresh;
}

void FileStructure::clear() {
    ui->tree->clear();
    _document_id = no_document;
}

void FileStructure::update( int id , const QString& filepath ) {

    _document_id = id;

    if ( filepath.isEmpty() ){
        ui->tree->clear();
        return;
    }


    try {



        QProcess p;
        p.setWorkingDirectory( QDir::currentPath() );
        p.start("/usr/bin/python3", { _py_script  , filepath}, QIODevice::ReadOnly );
        p.waitForFinished();
        QTextStream txt(&p);
        std::string raw_xml = txt.readAll().toStdString();

        if ( !raw_xml.empty() ){

            ui->tree->clear();

            xml::xml_document<char> doc;
            doc.parse<xml::parse_default>(  raw_xml.data() );

            ParseTree{}.parse(doc,ui->tree);

            ui->tree->expandAll();
        }


    }catch( const std::exception& e ){
        qDebug() << QString::fromStdString( e.what() );
    }
}




void FileStructure::on_actionToggle_triggered()
{
    QModelIndex index = ui->tree->currentIndex();

    if ( !index.isValid() )
        return;

    if ( ui->tree->isExpanded(index) )
        ui->tree->collapse(index);
    else
        ui->tree->expand(index);
}



}// widgets
