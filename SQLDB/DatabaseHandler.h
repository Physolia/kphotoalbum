/*
  Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>

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

#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include "DatabaseAddress.h"
#include "DatabaseConnection.h"
#include <kexidb/driver.h>
#include <kexidb/connectiondata.h>

namespace SQLDB
{
    class DatabaseHandler
    {
    public:
        DatabaseHandler(const DatabaseAddress& address);
        ~DatabaseHandler();

        DatabaseConnection& databaseConnection();

    protected:
        void connect();
        void openDatabase(const QString& name);
        void createAndOpenDatabase(const QString& name);
        void insertInitialData();

    private:
        static KexiDB::DriverManager* _driverManager;

        QString _databaseName;
        KexiDB::ConnectionData _connectionData;
        KexiDB::Driver* _driver;
        KexiDB::Connection* _connection;
    };
}

#endif /* DATABASEHANDLER_H */
