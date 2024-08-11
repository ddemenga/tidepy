#include <QProcess>
#include <QDir>
#include <QTextStream>
#include <QFont>
#include <QRegExp>

#include <QDebug>



#include "search_widget.h"
#include "ui_search_widget.h"



namespace widgets {




SearchWidget::SearchWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchWidget)
{
    ui->setupUi(this);
    ui->case_insensistive->setChecked(true);
    ui->all_files->setChecked(true);

    QFont mono = QFont("Monospace");
    mono.setFixedPitch(true);
    mono.setStyleHint(QFont::Monospace);
    ui->results->setFont(mono); // font must be monospace
    ui->pattern->setFont(mono);
    ui->replace->setFont(mono);

    ui->case_insensistive->setAutoExclusive(false);
    ui->word_only->setAutoExclusive(false);
    ui->regexp->setAutoExclusive(false);
    ui->all_files->setAutoExclusive(false);

    //ui->regexp->hide();
    ui->word_only->hide();
    ui->regexp->setChecked(true);

    ui->find->setDefaultAction(ui->actionFind);
    ui->replace->setDefaultAction(ui->actionReplace);


    _search_dir.setPath( QDir::currentPath() );
    _current_file = "";

    connect( ui->pattern , &QLineEdit::editingFinished , [&](  ) { ui->actionFind->trigger(); });
    connect( ui->results , &TESearchResult::file_selected , this , &SearchWidget::file_selected );
    connect( ui->results , &TESearchResult::file_changed , this , &SearchWidget::file_changed );

}

SearchWidget::~SearchWidget()
{
    delete ui;
}


void SearchWidget::set_search_dir(const QDir & value ) {

    _search_dir = value ;
}

void SearchWidget::set_current_file( const QFileInfo& path) {
    if ( path.exists() )
        _current_file = path.absoluteFilePath();
}

void SearchWidget::on_actionFind_triggered()
{


    if ( ui->pattern->text().isEmpty() )
        return;

    ui->results->clear();

    // colors
    QTextCursor tc = ui->results->textCursor();
    tc.setCharFormat(QTextCharFormat());
    QTextCharFormat default_format = tc.charFormat();
    QTextCharFormat filename_format = tc.charFormat();
    QTextCharFormat number_format = tc.charFormat();
    QTextCharFormat source_format = tc.charFormat();

    filename_format.setForeground(QColor(147, 93, 171).darker(100));
    number_format.setForeground(QColor(98, 177, 32).darker(100));
    source_format.setForeground(QColor(240, 82, 79).darker(100));


    // Build the query for grep/egrep
    QString opts="n", extra_opts ,pattern=ui->pattern->text();



    if ( ui->case_insensistive->isChecked() ){
        opts.append("i");
        _regexp_pattern = pattern;
        _regexp_pattern.append("|").append(pattern.toLower());


    }else{
        _regexp_pattern = pattern;
    }
    _regexp_pattern = QString("(%1)").arg(_regexp_pattern);

    if ( ui->all_files->isChecked() || _current_file.isEmpty() ){ // grep can't run without filename
        opts.append("R");
    }

    if ( ui->word_only->isChecked() && !ui->regexp->isChecked() ){
        extra_opts = "-e";
        pattern = QString("\\b%1\\b").arg(pattern);
        _regexp_pattern = QString("\b%1\b").arg(_regexp_pattern);
    }

    if ( ui->regexp->isChecked() ){
        extra_opts = "-e";
    }


    opts.insert(0,'-');

    //qDebug() << "grep --include=\"*.py\"" << opts << " " << extra_opts << " " << pattern;
    //qDebug() << "regexp " << regexp_pattern;



    //QString grep = QString("%1 --include=*.py %2 %3").arg(opts).arg(pattern).arg(_current_file);
    QStringList args({"--include=*.py","--with-filename"});
    args << opts ;
    args << pattern;
    if ( !opts.contains("R"))
        args << _current_file;
    else
        args << _search_dir.absolutePath();


    QProcess p;
    p.setWorkingDirectory( _search_dir.absolutePath() );

    if ( extra_opts.isEmpty() ){
        //qDebug() << "grep " << args ;
        tc.insertText( "grep " , filename_format);
        for (auto& a : args )
            tc.insertText( QString(" %1 ").arg(a),default_format);
        tc.insertText( "\n");
        p.start("grep", args , QIODevice::ReadOnly );
    }else{
        //qDebug() << "egrep " << args ;
        tc.insertText( "egrep " , filename_format);
        for (auto& a : args )
            tc.insertText( QString(" %1 ").arg(a) ,default_format);
        tc.insertText( "\n");
        p.start("egrep", args , QIODevice::ReadOnly );
    }

    p.waitForFinished();
    QTextStream txt(&p);

    for ( QString& s : txt.readAll().split("\n") ){

        if ( s.isEmpty() )
            break;


        // Search on a single file missing the filename + sep at the beginning
        //if ( !opts.contains("R"))
        //    s = QString("%1 : %2").arg(_current_file,s);

        int first = s.indexOf(':');
        int second = s.indexOf(':',first+1);

        tc.insertText( s.left(first) , filename_format);
        tc.insertText( "  :  ");
        tc.insertText(  s.mid(first+1 , second - 1 - first ) ,number_format );
        tc.insertText( "  :  ");
        //tc.insertText( source , default_format );


        QString source = s.mid(second + 1);
        QRegExp re(_regexp_pattern);
        re.setCaseSensitivity( ui->case_insensistive->isChecked() ? Qt::CaseInsensitive : Qt::CaseSensitive );

        int pos =0,last_pos = 0;
        while ((pos = re.indexIn(source, pos)) != -1 ) {

            int dpos = re.matchedLength();

            assert( dpos > 0 );

            if ( pos > last_pos )
                tc.insertText( source.mid( last_pos , pos - last_pos ) , default_format );

            tc.insertText( source.mid( pos, dpos ) , source_format );


            pos += dpos;
            last_pos = pos;
        }

        if ( last_pos < source.size() )
            tc.insertText( source.mid( last_pos ).trimmed() , default_format );

        tc.insertText("\n");

    }




}


void SearchWidget::on_actionReplace_triggered()
{
    ui->results->replace_all( _regexp_pattern, ui->replace_value->text() , ui->case_insensistive->isChecked() ?  false : true  );
}



} // widgets
