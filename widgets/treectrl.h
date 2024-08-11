#ifndef TREECTRL_H
#define TREECTRL_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QList>

namespace widgets {


class TreeCtrl : public QTreeView
{
    Q_OBJECT

public:

    explicit TreeCtrl(QWidget *parent = nullptr);
    virtual ~TreeCtrl();

    void set_columns(const QStringList& );

    bool is_empty() const ;


    QStandardItem* item( const QModelIndex& ) const ;

    QModelIndex add_item( const QString& text , const QModelIndex& parent = QModelIndex());

    QModelIndex add_item( const QString& text , const QIcon& ,  const QModelIndex& parent = QModelIndex() );

    template< typename Func , typename ...Args >
    void for_each( const QModelIndex& index , const Func& func , Args& ...args ){

        func(index,std::forward<Args&>(args)... );

        for ( int i=0; i < _model->rowCount( index ); i++ ){

            for_each( _model->index(i,0,index) , func  , std::forward<Args&>(args)... );

        }


    }

    template< typename Func >
    QModelIndex find( const QModelIndex& index , const Func& func ){

        if ( func( index ) )
            return index;

        for ( int i=0; i < _model->rowCount( index ); i++ ){

            auto found = find( _model->index(i,0,index) , func );
            if ( found.isValid() )
                return found;
        }

        return QModelIndex();
    }


    QList< QModelIndex > top_levels() const ;



protected:

    QStandardItemModel* _model;



};



} // widget



#endif // TREECTRL_H
