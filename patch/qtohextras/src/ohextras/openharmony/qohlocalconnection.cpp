/* ***************************************************************************
 *
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#include "qohlocalconnection.h"
#include <private/qobject_p.h>
#include <QtCore/qjsmodule.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qjsobject.h>
#include <private/qjspromise_p.h>
#include <QtCore/qopenharmony.h>
#include <private/qopenharmony_p.h>
#include <QtCore/qnapi.h>
#include "qohabstractlocalsocket_p.h"

QT_BEGIN_NAMESPACE

class QOhLocalConnectionPrivate : public QOhAbstractLocalSocketPrivate
{
    Q_DECLARE_PUBLIC(QOhLocalConnection)
};

QOhLocalConnection::QOhLocalConnection(QObject *parent)
    : QOhAbstractLocalSocket(*new QOhLocalConnectionPrivate, parent)
{

}

QOhLocalConnection::QOhLocalConnection(QJsObject *connection)
   : QOhAbstractLocalSocket(*new QOhLocalConnectionPrivate, nullptr)
{
    Q_D(QOhLocalConnection);
    d->m_object.reset(connection);
    d->onMessage();
    d->onError();
}

QT_END_NAMESPACE
