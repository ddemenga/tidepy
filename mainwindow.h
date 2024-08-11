#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "py_editor_tools/debugger_json_reader.h"
namespace dbg = debugger;

#include "widgets/debugger_console.h"
#include "widgets/line_pos_widgets.h"
#include "widgets/frames_widget.h"
#include "widgets/vars_widget.h"
#include "widgets/documentsviewer.h"
#include "widgets/filestructure.h"
#include "widgets/search_widget.h"
#include "widgets/python_settings_widget.h"
#include "widgets/projecttreemanager_widget.h"



#include "debugger_files.h"


namespace Ui {
class MainWindow;
}


namespace widgets {


class DockWrapper;





/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    /**
     * @brief MainWindow
     * @param parent
     */
    explicit MainWindow(QWidget *parent = nullptr);


    ~MainWindow();

    void load_settings();
    void save_settings();



signals:

    /**
     * @brief exec
     * @param module
     * @param args
     * @param breaks
     */
    void exec( const QString& module , const QStringList& args, const QStringList& breaks );



protected:

    /**
     * @brief closeEvent
     * @param event
     */
    void closeEvent(QCloseEvent *event) override;

private slots:


    void __set_enable_project_tools__( bool );

    /**
     * @brief __on_dir_deleted__
     * @param path
     */
    void __on_dir_deleted__(const QString & path );

    /**
     * @brief __on_file_deleted__
     * @param path
     */
    void __on_file_deleted__(const QString & path );

    /**
     * @brief __on_item_renamed__
     */
    void __on_item_renamed__( const QString& , const QString& );


    /**
     * @brief on_actionRun_triggered
     */
    void on_actionRun_triggered();

    /**
     * @brief on_actionDebug_triggered
     */
    void on_actionDebug_triggered();

    /**
     * @brief on_actionShow_whitespaces_triggered
     */
    void on_actionShow_whitespaces_triggered();

    /**
     * @brief on_actionAbout_Qt_triggered
     */
    void on_actionAbout_Qt_triggered();

    /**
     * @brief on_actionSettings_triggered
     */
    void on_actionSettings_triggered();

    /**
     * @brief on_actionOpenFile_triggered
     */
    void on_actionOpenFile_triggered();

    void on_actionNewProject_triggered();

    void on_actionSave_triggered();




    void on_actionTabSize1_triggered();

    void on_actionTabSize2_triggered();

    void on_actionTabSize4_triggered();

    void on_actionTabSize8_triggered();

    void on_actionTabSize16_triggered();

private:


    /**
     * @brief get_breakpoints
     * @return
     */
    debugger::QBreakInfoList get_breakpoints();




private:

    Ui::MainWindow *ui;


    // widgets
    LineColPosWidget* _pos_status;
    DebuggerConsole* _debugger_console;
    FramesWidget* _frames_stack;
    VarsWidget* _vars;
    ProjectTreeManager* _project_tree;
    FileStructure* _py_structure;
    SearchWidget* _search;
    PythonSettings* _settings;

    // docks for widgets
    DockWrapper* _dock_py_structure;
    DockWrapper* _dock_debugger_console;
    DockWrapper* _dock_frames_stack;
    DockWrapper* _dock_vars;
    DockWrapper* _dock_project_tree;
    DockWrapper* _dock_search;

    /**
     * @brief _dbg_files Copy Debugger Python implementation to share local directory
     */
    DebuggerFiles _dbg_files;
};



} // widgets

#endif // MAINWINDOW_H
