/*!
 * \file utils.cpp
 * \author Danilo Treffiletti <urban82@gmail.com>
 * \date 2014-07-17
 * \brief Application global utility functions.
 * \copyright 2014 (c) Danilo Treffiletti
 *
 *    This file is part of BeurerScaleManager.
 *
 *    BeurerScaleManager is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    BeurerScaleManager is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with BeurerScaleManager.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils.hpp"

#include <QtCore/QDebug>
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtCore/QLibraryInfo>
#include <QtCore/QDir>

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

#include <config.hpp>

// For the createTable functions
#include <Data/UserDataDB.hpp>

//! Table name for version table
#define VERSION_TABLE_NAME "TablesVersions"

namespace BSM {
namespace Utils {

QSqlDatabase db;

void loadTranslation()
{
    // '-' is added to default delimiters because it is used on Mac OS X instead of '_'.
    const QString searchDelimiters(QLatin1String("_.-"));

    // Get name of locale
    QString localeName = QLocale::system().name();
    qDebug() << "Loading translation for" << localeName;

    // Get translation for qt dialogues
    QTranslator* qtTr = new QTranslator(qApp);
    if (!localeName.startsWith(QLatin1String("en")) &&
        !qtTr->load("qt_" + localeName, QLibraryInfo::location(QLibraryInfo::TranslationsPath), searchDelimiters) &&
        !qtTr->load("qt_" + localeName, "translations", searchDelimiters)
    ) {
        qWarning() << "Cannot load QT translation for" << localeName;
    }
    qApp->installTranslator(qtTr);

    // Get translation for application
    QTranslator* bsmTr = new QTranslator(qApp);
    if (!localeName.startsWith(QLatin1String("en")) &&
        !bsmTr->load(localeName, BSM_CFG_TRANSLATIONS_PATH, searchDelimiters) &&
        !bsmTr->load(localeName, "translations", searchDelimiters)
    ) {
        qWarning() << "Cannot load translation for" << localeName;
    }
    qApp->installTranslator(bsmTr);
}

bool checkUserDirectory()
{
    // User folder
    QDir path = QDir::home();
    if (!path.exists()) {
        qCritical() << "Cannot find user directory" << path.path();
        QMessageBox::critical(0,
                              "Beurer Scale Manager - " + qApp->translate("BSM::Utils", "Directory not found"),
                              qApp->translate("BSM::Utils", "Cannot find user directory \"%1\".<br><br>Please check your environment.").arg(path.path())
        );
        return false;
    }
    qDebug() << "User directory" << path.path() << "OK";

    if (!path.exists(BSM_SAVING_FOLDER)) {
        qDebug() << "Try to create" << path.filePath(BSM_SAVING_FOLDER);
        if (!path.mkdir(BSM_SAVING_FOLDER)) {
            qCritical() << "Cannot create saving directory";
            QMessageBox::critical(0,
                                  "Beurer Scale Manager - " + qApp->translate("BSM::Utils", "Directory not created"),
                                  qApp->translate("BSM::Utils", "Cannot create user saving directory \"%1\".<br><br>Please check your environment.").arg(path.filePath(BSM_SAVING_FOLDER))
            );
            return false;
        }
    }
    if (!path.cd(BSM_SAVING_FOLDER)) {
        qCritical() << "Cannot open saving directory";
        QMessageBox::critical(0,
                              "Beurer Scale Manager - " + qApp->translate("BSM::Utils", "Directory not opened"),
                              qApp->translate("BSM::Utils", "Cannot open user saving directory \"%1\".<br><br>Please check your environment.").arg(path.filePath(BSM_SAVING_FOLDER))
        );
        return false;
    }
    qDebug() << "User saving directory" << path.path() << "OK";

    return true;
}

QString getSavingDirectory()
{
    return QDir::homePath() + "/" BSM_SAVING_FOLDER "/";
}

bool openDdAndCheckTables()
{
    // DB path
    QString dbPath = getSavingDirectory() + "BeurerScaleManager.db";
    qDebug() << "DB in" << dbPath;

    // Open DB
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        qCritical() << "Cannot open DB";
        QMessageBox::critical(0,
                              "Beurer Scale Manager - " + qApp->translate("BSM::Utils", "Database not opened"),
                              qApp->translate("BSM::Utils", "Cannot open the database \"%1\".<br><br>Please check your environment.").arg(dbPath)
        );
        return false;
    }

    // Create version table, if doesn't exists
    if (!executeQuery("CREATE TABLE IF NOT EXISTS " VERSION_TABLE_NAME " (tableName TEXT PRIMARY KEY, version INTEGER) WITHOUT ROWID;")) {
        qCritical() << "Cannot create version table";
        QMessageBox::critical(0,
                              "Beurer Scale Manager - " + qApp->translate("BSM::Utils", "Cannot create table"),
                              qApp->translate("BSM::Utils", "Cannot create table \"%1\".<br><br>Please check your environment.").arg(VERSION_TABLE_NAME)
        );
        return false;
    }

    // Check or create tables for objects
    QStringList failedTables;
    if (!Data::UserDataDB::createTable()) {
        qCritical() << "Cannot create table" << Data::UserDataDB::tableName;
        failedTables << Data::UserDataDB::tableName;
    }
    // Check for errors
    if (!failedTables.isEmpty()) {
        QMessageBox::critical(0,
                              "Beurer Scale Manager - " + qApp->translate("BSM::Utils", "Cannot create table"),
                              qApp->translate("BSM::Utils", "Cannot create the following tables: %1.<br><br>Please check your environment.").arg(failedTables.join(","))
        );
        return false;
    }

    return true;
}

void closeDb()
{
    db.close();
}

bool isTablePresent(const QString& tableName)
{
    return (db.tables().indexOf(tableName) >= 0);
}

bool createTable(const QString& tableName, const ColumnList& tableDefinition)
{
    QStringList tmp;
    ColumnList::ConstIterator it, itEnd = tableDefinition.end();
    for (it = tableDefinition.begin(); it != itEnd; ++it)
        tmp << it->first + " " + it->second;

    QString sql = QString("CREATE TABLE `%1` (%2) WITHOUT ROWID;").arg(tableName).arg(tmp.join(", "));
    qDebug() << "Creating table" << tableName << ":" << sql;
    return executeQuery(sql);
}

bool dropTable(const QString& tableName)
{
    if (!isTablePresent(tableName))
        return true;

    QSqlQuery query(db);

    if (executeQuery("DROP TABLE " + tableName + ";"))
        return true;

    qWarning() << "Cannot drop table" << tableName;
    return false;
}

int getTableVersion(const QString& tableName)
{
    if (!isTablePresent(VERSION_TABLE_NAME))
        return -1;
    if (!isTablePresent(tableName))
        return 0;

    QSqlQuery query(db);

    if (query.prepare("SELECT version FROM " VERSION_TABLE_NAME " WHERE tableName = :tableName;")) {
        query.bindValue(":tableName", tableName);
        if (query.exec()) {
            if (query.next()) {
                QVariant version = query.record().value("version");
                if (version.isValid()) {
                    bool ok;
                    int retval = version.toUInt(&ok);
                    if (ok)
                        return retval;
                }
            }
        }
    }

    qWarning() << "Cannot find version for table" << tableName;
    return -1;
}

bool setTableVersion(const QString& tableName, const int tableVersion)
{
    if (!isTablePresent(VERSION_TABLE_NAME))
        return false;
    if (!isTablePresent(tableName))
        return false;

    QSqlQuery query(db);

    if (query.prepare("INSERT OR REPLACE INTO TablesVersions (tableName, version) VALUES (:tableName, :version);")) {
        query.bindValue(":tableName", QVariant(tableName));
        query.bindValue(":version", tableVersion);
        if (query.exec()) {
            return true;
        }
    }

    qWarning() << "Cannot save version for table" << tableName;
    return false;
}

bool executeQuery(QString sql)
{
    QSqlQuery query(db);

    if (!query.prepare(sql))
        return false;
    if (!query.exec())
        return false;

    return true;
}

} // namespace Utils
} // namespace BSM
