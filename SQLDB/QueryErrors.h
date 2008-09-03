/*
  Copyright (C) 2006-2007 Tuomas Suutari <thsuut@utu.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/
#ifndef SQLDB_QUERYERRORS_H
#define SQLDB_QUERYERRORS_H

#include <qstring.h>
#include <QSqlError>
#include <QSqlQuery>

#define ERROR_CLASS(X,Y) \
struct X: public Y\
{\
    X(const QString& message=QString::null): Y(message) {}\
    QString name() const { return QString::fromLatin1(#X); }\
}

#define ERROR_CLASS2(X,Y) \
struct X: public Y\
{\
    X(const QString& queryLine=QString::null,\
      const QString& message=QString::null): Y(queryLine, message) {}\
    QString name() const { return QString::fromLatin1(#X); }\
}

namespace SQLDB
{
    class Error: public std::exception
    {
    public:
        Error(const QString& message=QString::null): _message(message) {}
        virtual ~Error() throw() {}
        const QString& message() const { return _message; }
        virtual QString name() const { return QString::fromLatin1("Error"); }
        virtual const char* what() const throw()
        {
            if (_whatString.isNull()) {
                _whatString = whatAsQString();
            }
            return _whatString.toLocal8Bit().constData();
        }
        virtual QString whatAsQString() const throw()
        {
            return name() + QLatin1String(": ") + message();
        }

    private:
        mutable QString _whatString;
        QString _message;
    };

    ERROR_CLASS(NotFoundError, Error);
    ERROR_CLASS(DriverNotFoundError, NotFoundError);
    ERROR_CLASS(DataNotFoundError, NotFoundError);
    ERROR_CLASS(RowNotFoundError, DataNotFoundError);
    ERROR_CLASS(EntryNotFoundError, DataNotFoundError);

    ERROR_CLASS(InitializationError, Error);
    ERROR_CLASS(DriverLoadError, InitializationError);

    ERROR_CLASS(ConnectionError, Error);
    ERROR_CLASS(ConnectionCreateError, ConnectionError);
    ERROR_CLASS(ConnectionOpenError, ConnectionError);
    ERROR_CLASS(ConnectionCloseError, ConnectionError);

    ERROR_CLASS(DatabaseError, Error);
    ERROR_CLASS(DatabaseCreateError, DatabaseError);
    ERROR_CLASS(DatabaseOpenError, DatabaseError);
    ERROR_CLASS(DatabaseSchemaError, DatabaseOpenError);

    ERROR_CLASS(TableCreateError, Error);

    ERROR_CLASS(TransactionError, Error);
    ERROR_CLASS(TransactionCreateError, TransactionError);
    ERROR_CLASS(TransactionCommitError, TransactionError);

    class SQLError: public Error
    {
    public:
        SQLError(const QString& queryLine=QString::null,
                 const QString& message=QString::null):
            Error(message), _queryLine(queryLine) {}
        virtual ~SQLError() throw() {}
        const QString& queryLine() const { return _queryLine; }
        QString name() const { return QString::fromLatin1("SQLError"); }
        virtual QString whatAsQString() const throw()
        {
            return
                Error::whatAsQString() + QLatin1String(". The query was \"") +
                this->queryLine() + QLatin1String("\"");
        }

    private:
        QString _queryLine;
    };

    ERROR_CLASS2(QueryError, SQLError);
    ERROR_CLASS2(StatementError, SQLError);

    class QtSQLError: public SQLError
    {
    public:
        QtSQLError(const QSqlError& qtError,
                   const QString& message):
            SQLError(QLatin1String("Unknown"), message),
            _qtError(qtError)
        {
            Q_ASSERT(_qtError.isValid());
        }
        QtSQLError(const QSqlQuery& executedQuery,
                   const QString& message=QString()):
            SQLError(executedQuery.lastQuery(), message),
            _qtError(executedQuery.lastError()),
            _bindings(executedQuery.boundValues().values())
        {
            Q_ASSERT(_qtError.isValid());
        }
        virtual ~QtSQLError() throw()
        {
        }
        virtual QString name() const { return QLatin1String("QtSQLError"); }
        virtual QString whatAsQString() const throw()
        {
            QStringList bindingsAsStrings;
            Q_FOREACH(QVariant x, _bindings)
                bindingsAsStrings.push_back(x.toString());
            return
                SQLError::whatAsQString() +
                QLatin1String(". Bindinds are (") +
                bindingsAsStrings.join(QLatin1String(",")) + QLatin1String(")") +
                QLatin1String(". The text of QSqlError is \"") +
                _qtError.text() + QLatin1String("\"");
        }

    private:
        QSqlError _qtError;
        QList<QVariant> _bindings;
    };
}

#undef ERROR_CLASS
#undef ERROR_CLASS2

#endif /* SQLDB_QUERYERRORS_H */
