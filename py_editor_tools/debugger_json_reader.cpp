#include "debugger_json_reader.h"

#include <QDebug>
#include <fstream>
#include <sstream>

namespace debugger {



Debugger_JsonReader::Debugger_JsonReader(QObject *parent)
    : QObject{parent}
{

}


QString Debugger_JsonReader::dump_var( int id , const QString& name  ){

    json data = json::array();

    json jvar;
    jvar["id"]= id;
    jvar["name"]= name.toStdString();

    data.insert( data.end() , jvar );

    std::stringstream ss; ss <<  data;
    return QString::fromStdString(ss.str());
}

QString Debugger_JsonReader::dump_vars( const QVarInfoList& vars ) {

    json data = json::array();
    for ( auto& var : vars ){

        json jvar;
        jvar["id"]=var.id;
        jvar["name"]=var.name.toStdString();

        data.insert( data.end() , jvar );
    }


    std::stringstream ss; ss <<  data;
    return QString::fromStdString(ss.str());

}

bool Debugger_JsonReader::read(const QString& raw_json ) {

    std::stringstream ss;
    ss << raw_json.toStdString();

    try {
        json data = json::parse(ss);

        auto it = data.find("reason");
        if ( it != data.end() ){

            std::string reason = *it;
            if ( reason == "break")
                return parse_break( data );
            else if ( reason == "set_break")
                return parse_set_break( data );
            else if ( reason == "frames")
                return parse_framestack( data );
            else if ( reason == "display")
                return parse_display( data );
            else if ( reason == "display_batch"){
                //qDebug() << "display_batch received:" << txt;
                return parse_display_batch( data );
            }else if ( reason == "display_expr"){
                return parse_display_expr( data );
            }else if ( reason == "exception")
                return parse_exception( data );

        }

    }catch( const json::parse_error& e ){

        qDebug() << "Parsing " << raw_json << ":\n" ;
    }

    return false;
}



bool Debugger_JsonReader::parse_break( const json& node ) {

    try {
        json stop = node["stop"];

        QString filename = QString::fromStdString( stop["filename"].template get<std::string>());
        std::string _n = stop["lineno"];
        int lineno = QString::fromStdString( _n ).toInt();

        emit go_to_line( filename , lineno );

    } catch( const std::exception& e ) {
        return false;
    }

    return true;


}

bool Debugger_JsonReader::parse_set_break( const json& node ) {

    try {

        auto it = node.find("error");
        if ( it != node.end() ){
            json error= *it;


            int lineno = error["lineno"];
            QString filename = QString::fromStdString( error["filename"].template get<std::string>());

            emit no_break_at( filename , lineno );

        }

        return true;


    } catch( const std::exception& e ) {
        return false;
    }

    return true;


}



bool Debugger_JsonReader::parse_framestack( const json& node ) {


    try {


        json stack = node["stack"];
        QFrameInfoList frames;

        for ( auto& frame : stack ){
            FrameInfo f;

            f.index = frame["index"];
            f.pid = frame["pid"];
            f.id = frame["id"];
            f.lineno = frame["lineno"];
            f.active = frame["active"].template get<std::string>() == "yes" ? true : false ;
            f.filename = QString::fromStdString( frame["filename"].template get<std::string>());
            f.func = QString::fromStdString( frame["func"].template get<std::string>());

            frames.append(  f );
        }

        emit framestack( frames );


    } catch( const std::exception& e ) {
        return false;
    }

    return true;


}


bool Debugger_JsonReader::parse_display( const json& node ) {

    try {

        json info  = node["info"];
        VarInfo var;
        var.id = info["id"];
        var.type = QString::fromStdString( info["type"].template get<std::string>());
        var.name = QString::fromStdString( info["name"].template get<std::string>());
        var.value = QString::fromStdString( info["value"].template get<std::string>());

        emit display_var( var );


    } catch( const std::exception& e ) {
        return false;
    }

    return true;
}



bool Debugger_JsonReader::parse_display_batch( const json& node ) {

    try {

        json list  = node["value"];

        QVarInfoList vars;
        VarInfo var;
        for ( auto& v  : list ){

            var.id = v["id"];
            var.type = QString::fromStdString( v["type"].template get<std::string>());
            var.name = QString::fromStdString( v["name"].template get<std::string>());
            var.value = QString::fromStdString( v["value"].template get<std::string>());

            vars.append(var);
        }

        emit display_vars( vars );


    } catch( const std::exception& e ) {
        return false;
    }

    return true;
}

bool Debugger_JsonReader::parse_display_expr( const json& node ) {

    try {

        json info  = node["expr"];
        std::string value( info["value"] );
        auto exp = QString::fromStdString( value );
        emit display_expr( exp );

    } catch( const std::exception& e ) {
        return false;
    }

    return true;
}


bool Debugger_JsonReader::parse_exception( const json& node ) {

    try {

        json stack  = node["stack"];
        QString value = QString::fromStdString( stack["value"].template get<std::string>());
        QStringList lines = value.split('\n');
        for(auto& line : lines)
            line.append("\n");
        emit exception( lines );


    } catch( const std::exception& e ) {
        return false;
    }

    return true;


}



} // debugger
