/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtEnginio module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "enginioqmlclient_p_p.h"
#include "Enginio/private/enginioclient_p.h"
#include "enginioqmlobjectadaptor_p.h"
#include "enginioqmlreply_p.h"

#include <QtNetwork/qnetworkreply.h>

#include <QtQml/qjsvalue.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqml.h>

#include <QDebug>

QT_BEGIN_NAMESPACE

/*!
  \qmltype EnginioClient
  \instantiates EnginioQmlClient
  \inqmlmodule Enginio 1
  \ingroup engino-qml

  \brief client interface to access Enginio
  \snippet simple.qml import

  EnginioClient is the heart of the QML API for Enginio.
  It is used for all communication with the Enginio backend.
  \l EnginioModel compliments it to make handling of multiple objects simple.

  The backend is identified by \l{backendId}{backend ID}.
  \snippet simple.qml client

  Once the backend is configured, it is possible to run queries by calling query on
  EnginioClient.
  For example to get all objects stored with the type "objects.image" run this query:
  \snippet simple.qml client-query

  EnginioClient gives you a convenient way to handle the responses to your queryies as well:
  \snippet simple.qml client-signals
*/

/*!
  \qmlproperty string Enginio1::EnginioClient::backendId
  Enginio backend ID. This can be obtained from the Enginio dashboard.
*/

/*!
  \qmlproperty url Enginio1::EnginioClient::serviceUrl
  \internal
  Enginio backend URL.

  Usually there is no need to change the default URL.
*/

/*!
  \qmlmethod EnginioReply Enginio1::EnginioClient::fullTextSearch(QJSValue query)
  \brief Perform a full text search on the database

  The \a query is an object sent to the backend to perform a fulltext search.
  Note that the search requires the searched properties to be indexed (on the server, configureable in the backend).

  \return EnginioReply containing the status and the result once it is finished.
  \sa EnginioReply, create(), query(), update(), remove(),
  {https://engin.io/documentation/rest/parameters/fulltext_query}{JSON request structure}
*/

/*!
  \qmlmethod EnginioReply Enginio1::EnginioClient::query(QJSValue query, Operation operation)
  \brief Query the database.

  The \a query is an object containing the actual query to the backend.
  The query will be run on the \a operation part of the backend.

  For example to find a usergroup named "myUsers" it is enough to call:
  \snippet socialtodos/Login.qml queryUsergroup

  \return EnginioReply containing the status and the result once it is finished.
  \sa EnginioReply, create(), update(), remove(), Enginio::operation
*/
/*!
  \qmlmethod EnginioReply Enginio1::EnginioClient::create(QJSValue query, Operation operation)
  \brief Create an object in the database.

  This command can create an object in the database, for example to create a new user
  such code could be used:
  \snippet users/Register.qml create

  To add a new member to a usergroup, the JSON needs to look like the example below.
  \code
  {
      "id": "groupId",
      "member": { "id": "abcd", "objectType": "users" }
  }
  \endcode

  \return EnginioReply containing the status and the result once it is finished.
*/
/*!
  \qmlmethod EnginioReply Enginio1::EnginioClient::update(QJSValue query, Operation operation)
  \brief Update an object in the database.

  To update access control list of an object the JSON loook like this:
  \code
  {
        "id": "objectId",
        "objectType": "objects.objectType",
        "access": { "read": ["id": "userId", "objectTypes": "users"],
                     "update": ["id": "userId", "objectTypes": "users"],
                     "admin": ["id": "userId", "objectTypes": "users"] }
  }
  \endcode

  \return EnginioReply containing the status of the query and the data once it is finished.
*/
/*!
  \qmlmethod EnginioReply Enginio1::EnginioClient::remove(QJSValue query, Operation operation)
  \brief Remove an object from the database.

  \return EnginioReply containing the status once it is finished.
*/

/*!
  \qmlmethod EnginioReply Enginio1::EnginioClient::uploadFile(QJSValue object, QUrl file)
  \brief Stores a \a file attached to an \a object in Enginio

  Each uploaded file needs to be associated with an object in the database.
  \note The upload will only work with the propper server setup: in the dashboard create a property
  of the type that you will use. Set this property to be a reference to files.

  In order to upload a file, first create an object:
  \snippet qmltests/tst_files.qml upload-create-object

  Then do the actual upload:
  \snippet qmltests/tst_files.qml upload

  Note: There is no need to directly delete files.
  Instead when the object that contains the link to the file gets deleted,
  the file will automatically be deleted as well.

  \sa downloadUrl()
*/

/*!
  \qmlmethod EnginioReply Enginio1::EnginioClient::downloadUrl(QJSValue object)
  \brief Get the download URL for a file

  \snippet qmltests/tst_files.qml download

  The response contains the download URL and the duration how long the URL will be valid.
  \code
    downloadReply.data.expiringUrl
    downloadReply.data.expiresAt
  \endcode

  \sa uploadFile()
*/

/*!
  \qmlsignal Enginio1::EnginioClient::finished(QJSValue reply)
  This signal is emitted when a \a reply finishes.

  \note that this signal is alwasy emitted, independent of whether
  the reply finished successfully or not.
*/

/*!
  \qmlsignal Enginio1::EnginioClient::error(QJSValue reply)
  This signal is emitted when a \a reply finishes and contains an error.
*/

EnginioQmlClient::EnginioQmlClient(QObject *parent)
    : EnginioClientConnection(*new EnginioQmlClientPrivate, parent)
{
    Q_D(EnginioQmlClient);
    d->init();
}

EnginioQmlClient::~EnginioQmlClient()
{}

void EnginioQmlClientPrivate::init()
{
    Q_Q(EnginioQmlClient);
    qRegisterMetaType<EnginioQmlClient*>();
    qRegisterMetaType<EnginioQmlReply*>();
    QObject::connect(q, &EnginioQmlClient::sessionTerminated, AuthenticationStateTrackerFunctor(this));
    QObject::connect(q, &EnginioQmlClient::sessionAuthenticated, AuthenticationStateTrackerFunctor(this, Enginio::Authenticated));
    QObject::connect(q, &EnginioQmlClient::sessionAuthenticationError, AuthenticationStateTrackerFunctor(this, Enginio::AuthenticationFailure));
}

EnginioQmlReply *EnginioQmlClient::fullTextSearch(const QJSValue &query)
{
    Q_D(EnginioQmlClient);

    ObjectAdaptor<QJSValue> o(query, d);
    QNetworkReply *nreply = d->query<QJSValue>(o, Enginio::SearchOperation);
    EnginioQmlReply *ereply = new EnginioQmlReply(d, nreply);
    return ereply;
}

EnginioQmlReply *EnginioQmlClient::query(const QJSValue &query, const Enginio::Operation operation)
{
    Q_D(EnginioQmlClient);

    ObjectAdaptor<QJSValue> o(query, d);
    QNetworkReply *nreply = d->query<QJSValue>(o, operation);
    EnginioQmlReply *ereply = new EnginioQmlReply(d, nreply);
    return ereply;
}

EnginioQmlReply *EnginioQmlClient::create(const QJSValue &object, const Enginio::Operation operation)
{
    Q_D(EnginioQmlClient);

    if (!object.isObject())
        return 0;

    ObjectAdaptor<QJSValue> o(object, d);
    QNetworkReply *nreply = d->create<QJSValue>(o, operation);
    EnginioQmlReply *ereply = new EnginioQmlReply(d, nreply);

    return ereply;
}

EnginioQmlReply *EnginioQmlClient::update(const QJSValue &object, const Enginio::Operation operation)
{
    Q_D(EnginioQmlClient);

    if (!object.isObject())
        return 0;

    ObjectAdaptor<QJSValue> o(object, d);
    QNetworkReply *nreply = d->update<QJSValue>(o, operation);
    EnginioQmlReply *ereply = new EnginioQmlReply(d, nreply);

    return ereply;
}

EnginioQmlReply *EnginioQmlClient::remove(const QJSValue &object, const Enginio::Operation operation)
{
    Q_D(EnginioQmlClient);

    if (!object.isObject())
        return 0;

    ObjectAdaptor<QJSValue> o(object, d);
    QNetworkReply *nreply = d->remove<QJSValue>(o, operation);
    EnginioQmlReply *ereply = new EnginioQmlReply(d, nreply);

    return ereply;
}

EnginioQmlReply *EnginioQmlClient::downloadUrl(const QJSValue &object)
{
    Q_D(EnginioQmlClient);

    if (!object.isObject())
        return 0;

    ObjectAdaptor<QJSValue> o(object, d);
    QNetworkReply *nreply = d->downloadUrl<QJSValue>(o);
    EnginioQmlReply *ereply = new EnginioQmlReply(d, nreply);

    return ereply;
}

EnginioQmlReply *EnginioQmlClient::uploadFile(const QJSValue &object, const QUrl &url)
{
    Q_D(EnginioQmlClient);

    if (!object.isObject())
        return 0;

    ObjectAdaptor<QJSValue> o(object, d);
    QNetworkReply *nreply = d->uploadFile<QJSValue>(o, url);
    EnginioQmlReply *ereply = new EnginioQmlReply(d, nreply);

    return ereply;
}

QByteArray EnginioQmlClientPrivate::toJson(const QJSValue &value)
{
    (void)jsengine();
    return _stringify.call(QJSValueList() << value).toString().toUtf8();
}

QJSValue EnginioQmlClientPrivate::fromJson(const QByteArray &value)
{
    (void)jsengine();
    return _parse.call(QJSValueList() << jsengine()->toScriptValue(value));
}

void EnginioQmlClientPrivate::_setEngine()
{
    QJSEngine *engine = QtQml::qmlEngine(q_ptr);
    _engine = engine;
    _stringify = engine->evaluate("JSON.stringify");
    _parse = engine->evaluate("JSON.parse");
    Q_ASSERT(_stringify.isCallable());
    Q_ASSERT(_parse.isCallable());
}

void EnginioQmlClientPrivate::emitSessionTerminated() const
{
    Q_Q(const EnginioQmlClient);
    emit q->sessionTerminated();
}

void EnginioQmlClientPrivate::emitSessionAuthenticated(EnginioReplyState *reply)
{
    Q_Q(EnginioQmlClient);
    emit q->sessionAuthenticated(jsengine()->newQObject(reply));
}

void EnginioQmlClientPrivate::emitSessionAuthenticationError(EnginioReplyState *reply)
{
    Q_Q(EnginioQmlClient);
    emit q->sessionAuthenticationError(jsengine()->newQObject(reply));
}

void EnginioQmlClientPrivate::emitFinished(EnginioReplyState *reply)
{
    Q_Q(EnginioQmlClient);
    emit q->finished(jsengine()->newQObject(reply));
}

void EnginioQmlClientPrivate::emitError(EnginioReplyState *reply)
{
    Q_Q(EnginioQmlClient);
    emit q->error(jsengine()->newQObject(reply));
}

EnginioReplyState *EnginioQmlClientPrivate::createReply(QNetworkReply *nreply)
{
    return new EnginioQmlReply(this, nreply);
}

QT_END_NAMESPACE
