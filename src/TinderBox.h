#pragma once

#include <vector>

#include <QException>
#include <QDebug>
#include <QMessageBox>
#include <QSharedPointer>
#include <QStringList>

#include "pugixml/pugixml.hpp"

class TinderBoxExc : public QException {
public:
    TinderBoxExc() throw() {}
	TinderBoxExc( const TinderBoxExc &obj ) throw() : QException( obj ), mMsg( obj.mMsg ) {}
    TinderBoxExc( const QString &msg ) throw() : mMsg( msg ) {}
    ~TinderBoxExc() throw() {}
    QString msg() const throw() { return mMsg; }
    const char * what() const throw() { return mMsg.toStdString().c_str(); }
    void raise() const { throw *this; }
	QException * clone() const { return new TinderBoxExc( *this ); }

private:
    QString mMsg;
};

// Exception declare macro
#define TBOX_EXC_DECL( _DerivedExc_, _BaseExc_ )        \
    class _DerivedExc_ : public _BaseExc_ {             \
    public:                                             \
        _DerivedExc_( const QString &msg ) throw()      \
        : _BaseExc_( msg + "(" #_DerivedExc_ ")" ) {}   \
    };

TBOX_EXC_DECL( TemplateXmlLoadFailed, TinderBoxExc )
TBOX_EXC_DECL( GenerateFailed, TinderBoxExc )
