#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyle>


#include "documentsviewer.h"
#include "ui_documentsviewer.h"

#include "editor/lang_highlighters/hightlighter_python.h"


#include "editor/filters/filters.h"



namespace widgets {






DocumentsViewer::DocumentsViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DocumentsViewer)
{
    ui->setupUi(this);
}




DocumentsViewer::~DocumentsViewer()
{
    delete ui;

}


int DocumentsViewer::num_documents() const {
    return ui->documents->count();
}

te::TextEditor* DocumentsViewer::current_document() {

    auto* widget =  ui->documents->currentWidget();
    if ( widget )
        return qobject_cast<text_editor::TextEditor*>(widget);
    return nullptr;
}


void DocumentsViewer::on_documents_currentChanged(int /*index*/)
{
    auto* current_editor = qobject_cast<text_editor::TextEditor*>( ui->documents->currentWidget() );
    for_each_document( [current_editor](  text_editor::TextEditor* e ){
        if ( e != current_editor )
            e->set_active(false);
    } );

    if ( current_editor )
        current_editor->set_active(true);

    emit document_changed( current_editor );
}



te::TextEditor* DocumentsViewer::operator[](const QString &path){

    static int ids = 0;

    QFileInfo info(path);

    te::TextEditor* editor =  find( path );
    if ( !editor ){


        if ( _cache.size() ){
            editor = _cache.takeFirst();
            editor->clear();

        } else {

            editor = new text_editor::TextEditor();

            editor->set_id( ++ids  );

            // connect the signals just once
            connect( editor , &text_editor::TextEditor::cursor_pos_changed , this , &DocumentsViewer::__on_document_cursor_pos_changed__ );
            connect( editor , &text_editor::TextEditor::filename_changed , this , &DocumentsViewer::__on_document_filename_changed__ );
            connect( editor , &text_editor::TextEditor::remove_document, this , &DocumentsViewer::__on_document_remove__ );
            connect( editor , &text_editor::TextEditor::breaks_changed, this , &DocumentsViewer::breaks_changed );
            connect( editor , &text_editor::TextEditBase::state_changed, this , &DocumentsViewer::__on_editor_state_changed__ );

            editor->add_snippet( _class_snippet, new text_editor::filters::ClassFilter ,QKeySequence(tr("Ctrl+Shift+1")) );
            editor->add_snippet( _member_function_snippet , new text_editor::filters::MemberFunctionFilter ,QKeySequence(tr("Ctrl+Shift+2")) );
            editor->add_snippet( _property_snippet , new text_editor::filters::DefProperty ,QKeySequence(tr("Ctrl+Shift+3")) );
            editor->add_snippet( _function_snippet, new text_editor::filters::FunctionFilter ,QKeySequence(tr("Ctrl+Shift+4")) );
            editor->add_snippet( _case_snippet  , new text_editor::filters::CaseFilter ,QKeySequence(tr("Ctrl+Shift+5")) );
            editor->add_snippet( _loop_list_snippet , new text_editor::filters::LoopOverSequence ,QKeySequence(tr("Ctrl+Shift+6")) );
            editor->add_snippet( _loop_dict_snippet , new text_editor::filters::LoopOverMap ,QKeySequence(tr("Ctrl+Shift+7")) );
        }

        editor->set_filename(path);

        te::TextEditorHighlighter teh;
        teh.sh = new te::SyntaxHighlighter(editor->document()); // attach the highlighter to QTextDocument
        teh.parser = new te::highlighter::PythonHighighter;
        teh.sh->set_highlighter(teh.parser);
        editor->set_highlighter( teh );

        if ( info.isFile())
            editor->load_file(path);
        editor->setDocumentTitle( QString("%1 [%2]").arg(info.baseName()).arg(editor->id()) );

        ui->documents->addTab( editor , editor->documentTitle());
    }


    int pos = ui->documents->indexOf( editor );
    ui->documents->setCurrentIndex( pos );
    ui->documents->setTabToolTip( pos , editor->filename().absoluteFilePath() );


    //editor->set_class_snippet( _class_snippet );

    /**
    qDebug() << "ctor----------------------";
    size_t i =0;
    for ( auto* w : _cache )
        qDebug() << i++ << ">> " << w->id() << ":" << w;
    **/

    return editor;

}

te::TextEditor* DocumentsViewer::find( const QString& filename ) {

    for  ( int ipos = 0; ipos < ui->documents->count(); ++ipos ){
        auto* editor = qobject_cast<text_editor::TextEditor*>( ui->documents->widget(ipos));
        assert( editor );
        if ( editor->filename().absoluteFilePath() == filename ){
            return editor;
        }
    }

    return nullptr;
}

te::TextEditor* DocumentsViewer::at( int pos ) {
    if ( 0<= pos && pos < ui->documents->count() )
        return qobject_cast<text_editor::TextEditor*>( ui->documents->widget(pos));
    return nullptr;
}

int DocumentsViewer::next(int pos ){

    if ( -1 < pos && (pos+1) < ui->documents->count() )
        return pos+1;
    return -1;
}


void DocumentsViewer::save_document(bool check_dirty) {

    auto* editor = qobject_cast<text_editor::TextEditor*>( ui->documents->currentWidget());
    assert( editor );

    save_document(editor,check_dirty);

}

void DocumentsViewer::save_as_document() {

    auto* editor = qobject_cast<text_editor::TextEditor*>( ui->documents->currentWidget());
    assert( editor );

    editor->save_as();

}



bool DocumentsViewer::save_document( text_editor::TextEditor * editor , bool check_dirty ) {


    if ( !editor )
        return true;

    if ( editor->is_dirty() ){

        if ( check_dirty ){
            if ( QMessageBox::question(this,"Save File","The file is not saved yet. Save it now ?") == QMessageBox::Yes )
                editor->save();
        }else
            editor->save();

        return true;
    }

    return false;
}



bool DocumentsViewer::save_and_close_all(){


    for  ( int ipos = 0, iend = ui->documents->count(); ipos < iend ; ++ipos ){

        auto* editor = qobject_cast<text_editor::TextEditor*>( ui->documents->widget(0));
        if ( editor->is_debugging() )
            return false ;
        editor->exit();
    }

    return true;


}

void DocumentsViewer::save_all(){

    for_each_document( []( auto* e ){
        e->save();
    });
}




void DocumentsViewer::__on_document_cursor_pos_changed__( int line , int col , int pos )
{

    auto* editor = qobject_cast<text_editor::TextEditor*>( ui->documents->currentWidget());
    if ( editor ){

        editor->do_extra_selections();
    }

    emit position_changed( line , col , pos );
}


void DocumentsViewer::__on_document_filename_changed__(int id, const QString&  path ) {

    for ( int ipos =0; ipos < ui->documents->count(); ipos++ ){

        auto* editor = qobject_cast<text_editor::TextEditor*>( ui->documents->widget(ipos) );
        assert( editor );
        if ( long(editor->id()) == id )
            ui->documents->setTabText( ipos , QFileInfo(path).baseName() );
    }
}


void DocumentsViewer::__on_editor_state_changed__( int id ){

    for ( int pos =0; pos < num_documents(); ++pos ){

        auto* e = at(pos);
        if ( e->id() == id ){
            if ( e->is_dirty() ){

                ui->documents->setTabIcon( pos , style()->standardIcon(QStyle::SP_DriveFDIcon) );

            } else {
                QIcon icon;
                ui->documents->setTabIcon( pos , icon);
            }
        }
    }
}

void DocumentsViewer::on_documents_tabCloseRequested(int index)
{
    auto* editor = qobject_cast<text_editor::TextEditor*>( ui->documents->widget(index));
    if ( editor->is_debugging())
        return;
    editor->exit();

}

void DocumentsViewer::__on_document_remove__( text_editor::TextEditor* w){


    int id = w->id();
    ui->documents->removeTab( ui->documents->indexOf(w) );

    emit document_removed(id);

    _cache.append(w);

    /**
    qDebug() << "dtor----------------------";
    size_t i =0;
    for ( auto* w : _cache )
        qDebug() << i++ << ">> " << w->id() << ":" << w;
    **/



}


}// widgets

