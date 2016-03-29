#include "services/databaseservice.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QSqlError>

DatabaseService::DatabaseService() {
}

/**
 * @brief Returns the path to the database (on disk)
 * @return
 */
QString DatabaseService::getDiskDatabasePath() {

    QStandardPaths::StandardLocation location;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    location = QStandardPaths::AppDataLocation;
#else
    location = QStandardPaths::GenericDataLocation;
#endif

    // get the path to store the database
    QString path = QStandardPaths::writableLocation(location);
    QDir dir;

    // create path if it doesn't exist yet
    dir.mkpath(path);

    return path + QDir::separator() + "LogAnalyzer.sqlite";
}

bool DatabaseService::removeDiskDatabase() {
    QFile file(getDiskDatabasePath());

    if (file.exists()) {
        qDebug() << __func__ << " - 'removing database file': "
            << file.fileName();
        return file.remove();
    }

    return false;
}

bool DatabaseService::createConnection() {
    return createMemoryConnection() && createDiskConnection();
}

bool DatabaseService::reinitializeDiskDatabase() {
    return removeDiskDatabase() && createDiskConnection() && setupTables();
}

bool DatabaseService::createMemoryConnection() {
    QSqlDatabase dbMemory = QSqlDatabase::addDatabase("QSQLITE", "memory");
    dbMemory.setDatabaseName(":memory:");

    if (!dbMemory.open()) {
        QMessageBox::critical(0, QWidget::tr("Cannot open memory database"),
              QWidget::tr(
                  "Unable to establish a database connection.\n"
                       "This application needs SQLite support. Please read "
                       "the Qt SQL driver documentation for information how "
                       "to build it.\n\n"
                       "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }

    return true;
}

bool DatabaseService::createDiskConnection() {
    QSqlDatabase dbDisk = QSqlDatabase::addDatabase("QSQLITE", "disk");
    dbDisk.setDatabaseName(getDiskDatabasePath());

    if (!dbDisk.open()) {
        QMessageBox::critical(0, QWidget::tr("Cannot open disk database"),
              QWidget::tr(
                      "Unable to establish a database connection.\n"
                          "This application needs SQLite support. Please read "
                          "the Qt SQL driver documentation for information how "
                          "to build it.\n\n"
                          "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }

    return true;
}

bool DatabaseService::setupTables() {
    QSqlDatabase dbDisk = QSqlDatabase::database("disk");
    QSqlQuery queryDisk(dbDisk);

    queryDisk.exec("CREATE TABLE appData ("
                           "name VARCHAR(255) PRIMARY KEY, "
                           "value VARCHAR(255));");
    int version = getAppData("database_version").toInt();
    qDebug() << __func__ << " - 'database_version': " << version;

    if (version < 1) {
        queryDisk.exec("CREATE TABLE logFileSource ("
                               "id INTEGER PRIMARY KEY,"
                               "type INTEGER,"
                               "name VARCHAR(255),"
                               "local_path VARCHAR(255),"
                               "ezp_server_url VARCHAR(255),"
                               "ezp_username VARCHAR(255),"
                               "ezp_password VARCHAR(255),"
                               "priority INTEGER DEFAULT 0)");
        version = 1;
    }

    setAppData("database_version", QString::number(version));

    return true;
}

bool DatabaseService::setAppData(QString name, QString value) {
    QSqlDatabase db = QSqlDatabase::database("disk");
    QSqlQuery query(db);

    query.prepare("REPLACE INTO appData ( name, value ) "
                          "VALUES ( :name, :value )");
    query.bindValue(":name", name);
    query.bindValue(":value", value);
    return query.exec();
}

QString DatabaseService::getAppData(QString name) {
    QSqlDatabase db = QSqlDatabase::database("disk");
    QSqlQuery query(db);

    query.prepare("SELECT value FROM appData WHERE name = :name");
    query.bindValue(":name", name);

    if (!query.exec()) {
        qCritical() << __func__ << ": " << query.lastError();
    } else if (query.first()) {
        return query.value("value").toString();
    }

    return "";
}
