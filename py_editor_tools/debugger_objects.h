#ifndef DEBUGGER_OBJECTS_H
#define DEBUGGER_OBJECTS_H

#include <QString>
#include <QList>
#include <QMetaType>


namespace debugger {


struct FrameInfo {

    int index;
    size_t pid;
    size_t id;
    int lineno;
    bool active;
    QString filename;
    QString func;
};

typedef QList<FrameInfo> QFrameInfoList;


struct VarInfo {

    int id ;
    QString type, name, value;
    bool updated;

};

typedef QList<VarInfo> QVarInfoList;



struct BreakInfo {

    int lineno ;
    QString filename;
    bool enabled;

};

typedef QList<BreakInfo> QBreakInfoList;



} // debugger



Q_DECLARE_METATYPE( debugger::FrameInfo )
Q_DECLARE_METATYPE( debugger::QFrameInfoList )

Q_DECLARE_METATYPE( debugger::VarInfo )
Q_DECLARE_METATYPE( debugger::QVarInfoList )

Q_DECLARE_METATYPE( debugger::BreakInfo )
Q_DECLARE_METATYPE( debugger::QBreakInfoList )

#endif // DEBUGGER_OBJECTS_H
