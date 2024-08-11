#ifndef DOCUMENTSVIEWER_H
#define DOCUMENTSVIEWER_H

#include <QWidget>
#include <QDir>
#include <functional>
#include <QAction>





#include "editor/editor.h"
namespace te = text_editor;


namespace Ui {
class DocumentsViewer;
}




namespace widgets {




class DocumentsViewer : public QWidget
{
    Q_OBJECT

public:
    explicit DocumentsViewer(QWidget *parent = nullptr);
    ~DocumentsViewer();



    int num_documents() const ;


    te::TextEditor* operator[](const QString& path );
    te::TextEditor* current_document();
    te::TextEditor* find( const QString& path );
    te::TextEditor* at(int pos );

    int  next( int pos );

    template< typename  Func>
    void for_each_document( const Func& op  ){
        for ( int pos =0; pos < num_documents(); ++pos ){
            auto* editor = at(pos);
            if ( editor )
                op( editor );
        }
    }
    //void for_each_document(const std::function<void (te::TextEditor *)> &op );


    inline void set_class_snippet( QAction* value ) { _class_snippet = value; }
    inline void set_member_function_snippet( QAction* value ) { _member_function_snippet = value; }
    inline void set_function_snippet( QAction* value ) { _function_snippet = value; }
    inline void set_case_snippet( QAction* value ) { _case_snippet = value; }
    inline void set_property_snippet( QAction* value ){ _property_snippet = value; }
    inline void set_loop_list_snippet( QAction* value ) { _loop_list_snippet =value; }
    inline void set_loop_dict_snippet( QAction* value ) { _loop_dict_snippet = value; }


public slots:

    void save_document(bool check_dirty = true );
    void save_as_document();
    void save_all();
    bool save_and_close_all();

signals:

    void document_added();
    void document_changed( text_editor::TextEditor* );
    void document_removed( int id );
    void position_changed( int line , int col , int pos );
    void breaks_changed( const QString& , const dbg::QBreakInfoList& );

private slots:
    void on_documents_currentChanged(int index);
    void on_documents_tabCloseRequested(int index);
    void __on_document_cursor_pos_changed__( int line , int col , int pos );
    void __on_document_filename_changed__(int id , const QString&  path);
    void __on_document_remove__( text_editor::TextEditor* );
    void __on_editor_state_changed__( int id );

private:

    te::TextEditor*  widget_by_filename( const QString& filename );


    bool save_document( text_editor::TextEditor* , bool check_dirty = true );

private:

    Ui::DocumentsViewer *ui;


    QAction* _class_snippet;
    QAction* _member_function_snippet;
    QAction* _function_snippet;
    QAction* _case_snippet;
    QAction* _property_snippet;
    QAction* _loop_list_snippet;
    QAction* _loop_dict_snippet;

    // The Editors widgets are reused
    QList<te::TextEditor*> _cache;
};


} // widgets

#endif // DOCUMENTSVIEWER_H
