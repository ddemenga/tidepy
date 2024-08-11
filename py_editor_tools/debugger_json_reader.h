#ifndef DEBUGGER_JSONREADER_H
#define DEBUGGER_JSONREADER_H

#include <QObject>


#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include  "debugger_objects.h"



namespace debugger {

/**
 * @brief The Debugger_JsonReader class
 *
 * Read data coming from the process  being debugged, in json format, parse it and send the resulting text to the debugger console.
 *
 */
class Debugger_JsonReader : public QObject
{
    Q_OBJECT

public:
    explicit Debugger_JsonReader(QObject *parent = nullptr);

public slots:

    /**
     * @brief read  : Main function. Called by DebuggerConsole to parse json text.
     * @param raw_xml
     * @return true if success, false otherwise.
     */
    bool read( const QString& raw_json );

    /**
     * @brief dump_var : Transform a (id,name) pair in a json text object.
     * @param id
     * @param name
     * @return the json text object.
     */
    QString dump_var( int id , const QString& name  );


    /**
     * @brief dump_vars : Transform a list of QVarInfo structure in json text object.
     * @param vars
     * @return the json text object
     */
    QString dump_vars( const QVarInfoList& vars );

signals:

    // Signals for the DebuggerConsole class.

    void go_to_line( const QString& filename,  int line  );
    void no_break_at(const QString& filename,  int line  );
    void framestack( const QFrameInfoList& );
    void display_var( const VarInfo& );
    void display_vars( const QVarInfoList& );
    void display_expr( const QString& );
    void exception( const QStringList& );

private:

    bool parse_break( const json& node );
    bool parse_set_break( const json& node );
    bool parse_framestack( const json& node );
    bool parse_exception( const json& node );
    bool parse_display( const json& node );
    bool parse_display_batch( const json& node);
    bool parse_display_expr( const json& node);
};


} // debugger
#endif // DEBUGGER_JSONREADER_H
