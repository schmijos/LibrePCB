/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
 * http://librepcb.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include "sqlitedatabase.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {

/*****************************************************************************************
 *  Class TransactionScopeGuard
 ****************************************************************************************/

SQLiteDatabase::TransactionScopeGuard::TransactionScopeGuard(SQLiteDatabase& db) throw (Exception) :
    mDb(db), mIsCommited(false)
{
    mDb.beginTransaction(); // can throw
}

void SQLiteDatabase::TransactionScopeGuard::commit() throw (Exception)
{
    mDb.commitTransaction(); // can throw
    mIsCommited = true;
}

SQLiteDatabase::TransactionScopeGuard::~TransactionScopeGuard() noexcept
{
    if (!mIsCommited) {
        try {
            mDb.rollbackTransaction(); // can throw
        } catch (Exception& e) {
            qCritical() << "Could not rollback database transaction:" << e.getUserMsg();
        }
    }
}

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

SQLiteDatabase::SQLiteDatabase(const FilePath& filepath) throw (Exception) :
    QObject(nullptr)//, mNestedTransactionCount(0)
{
    // create database
    mDb = QSqlDatabase::addDatabase("QSQLITE", filepath.toStr());
    mDb.setDatabaseName(filepath.toStr());

    // check if database is valid
    if (!mDb.isValid()) {
        throw RuntimeError(__FILE__, __LINE__, QString(),
            QString(tr("Invalid database: \"%1\"")).arg(filepath.toNative()));
    }

    // open the database
    if (!mDb.open()) {
        throw RuntimeError(__FILE__, __LINE__, QString(),
            QString(tr("Could not open database: \"%1\"")).arg(filepath.toNative()));
    }

    // set SQLite options
    exec("PRAGMA foreign_keys = ON"); // can throw

    // check if all required features are available
    Q_ASSERT(mDb.driver() && mDb.driver()->hasFeature(QSqlDriver::Transactions));
    Q_ASSERT(mDb.driver() && mDb.driver()->hasFeature(QSqlDriver::PreparedQueries));
    Q_ASSERT(mDb.driver() && mDb.driver()->hasFeature(QSqlDriver::LastInsertId));
}

SQLiteDatabase::~SQLiteDatabase() noexcept
{
    mDb.close();
}

/*****************************************************************************************
 *  SQL Commands
 ****************************************************************************************/

void SQLiteDatabase::beginTransaction() throw (Exception)
{
    //Q_ASSERT(mNestedTransactionCount >= 0);
    //if (mNestedTransactionCount == 0) {
        if (!mDb.transaction()) {
            throw RuntimeError(__FILE__, __LINE__, QString(),
                tr("Could not start database transaction."));
        }
    //}
    //mNestedTransactionCount++;
}

void SQLiteDatabase::commitTransaction() throw (Exception)
{
    //Q_ASSERT(mNestedTransactionCount >= 0);
    //if (mNestedTransactionCount == 1) {
        if (!mDb.commit()) {
            throw RuntimeError(__FILE__, __LINE__, QString(),
                tr("Could not commit database transaction."));
        }
    //} else if (mNestedTransactionCount == 0) {
    //    throw RuntimeError(__FILE__, __LINE__, QString(),
    //        tr("Cannot commit database transaction because no one is active."));
    //}
    //mNestedTransactionCount--;
}

void SQLiteDatabase::rollbackTransaction() throw (Exception)
{
    //Q_ASSERT(mNestedTransactionCount >= 0);
    //if (mNestedTransactionCount == 1) {
        if (!mDb.rollback()) {
            throw RuntimeError(__FILE__, __LINE__, QString(),
                tr("Could not rollback database transaction."));
        }
    //} else if (mNestedTransactionCount == 0) {
    //    throw RuntimeError(__FILE__, __LINE__, QString(),
    //        tr("Cannot rollback database transaction because no one is active."));
    //}
    //mNestedTransactionCount--;
}

void SQLiteDatabase::clearTable(const QString& table) throw (Exception)
{
    exec("DELETE FROM " % table); // can throw
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

QSqlQuery SQLiteDatabase::prepareQuery(const QString& query) const throw (Exception)
{
    QSqlQuery q(mDb);
    if (!q.prepare(query)) {
        throw RuntimeError(__FILE__, __LINE__, QString("%1: %2, %3").arg(query,
            q.lastError().databaseText(), q.lastError().driverText()),
            QString(tr("Error while preparing SQL query: %1")).arg(query));
    }
    return q;
}

int SQLiteDatabase::insert(QSqlQuery& query) throw (Exception)
{
    exec(query); // can throw

    bool ok = false;
    int id = query.lastInsertId().toInt(&ok);
    if (ok) {
        return id;
    } else {
        throw RuntimeError(__FILE__, __LINE__, query.lastQuery(),
            QString(tr("Error while executing SQL query: %1")).arg(query.lastQuery()));
    }
}

void SQLiteDatabase::exec(QSqlQuery& query) throw (Exception)
{
    if (!query.exec()) {
        throw RuntimeError(__FILE__, __LINE__, QString("%1: %2, %3").arg(query.lastQuery(),
            query.lastError().databaseText(), query.lastError().driverText()),
            QString(tr("Error while executing SQL query: %1")).arg(query.lastQuery()));
    }
}

void SQLiteDatabase::exec(const QString& query) throw (Exception)
{
    QSqlQuery q(query, mDb);
    exec(q);
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb

