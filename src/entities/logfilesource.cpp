#include "logfilesource.h"
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSettings>
#include <QDir>


LogFileSource::LogFileSource() {
    id = 0;
    name = "";
    localPath = "";
    type = 1;
    ezpServerUrl = "";
    ezpUsername = "";
    ezpPassword = "";
    priority = 0;
}

int LogFileSource::getId() {
    return this->id;
}

QString LogFileSource::getLocalPath() {
    return this->localPath;
}

QString LogFileSource::getEzpServerUrl() {
    return this->ezpServerUrl;
}

QString LogFileSource::getEzpUsername() {
    return this->ezpUsername;
}

QString LogFileSource::getEzpPassword() {
    return this->ezpPassword;
}

int LogFileSource::getType() {
    return this->type;
}

QString LogFileSource::getName() {
    return this->name;
}

int LogFileSource::getPriority() {
    return this->priority;
}

void LogFileSource::setName(QString text) {
    this->name = text;
}

void LogFileSource::setType(int id) {
    this->type = id;
}

void LogFileSource::setLocalPath(QString text) {
    this->localPath = text;
}

void LogFileSource::setEzpServerUrl(QString text) {
    this->ezpServerUrl = text;
}

void LogFileSource::setEzpUsername(QString text) {
    this->ezpUsername = text;
}

void LogFileSource::setEzpPassword(QString text) {
    this->ezpPassword = text;
}

void LogFileSource::setPriority(int value) {
    this->priority = value;
}

LogFileSource LogFileSource::fetch(int id) {
    QSqlDatabase db = QSqlDatabase::database("disk");
    QSqlQuery query(db);

    LogFileSource logFileSource;

    query.prepare("SELECT * FROM logFileSource WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << __func__ << ": " << query.lastError();
    } else if (query.first()) {
        logFileSource.fillFromQuery(query);
    }

    return logFileSource;
}

int LogFileSource::countAll() {
    QSqlDatabase db = QSqlDatabase::database("disk");
    QSqlQuery query(db);

    query.prepare("SELECT COUNT(*) AS cnt FROM logFileSource");

    if (!query.exec()) {
        qWarning() << __func__ << ": " << query.lastError();
    } else if (query.first()) {
        return query.value("cnt").toInt();
    }

    return 0;
}

bool LogFileSource::localPathExists() {
    QDir folder(localPath);
    return folder.exists() && !localPath.isEmpty();
}

bool LogFileSource::remove() {
    QSqlDatabase db = QSqlDatabase::database("disk");
    QSqlQuery query(db);

    query.prepare("DELETE FROM logFileSource WHERE id = :id");
    query.bindValue(":id", this->id);

    if (!query.exec()) {
        qWarning() << __func__ << ": " << query.lastError();
        return false;
    } else {
        return true;
    }
}

LogFileSource LogFileSource::logFileSourceFromQuery(QSqlQuery query) {
    LogFileSource logFileSource;
    logFileSource.fillFromQuery(query);
    return logFileSource;
}

bool LogFileSource::fillFromQuery(QSqlQuery query) {
    this->id = query.value("id").toInt();
    this->name = query.value("name").toString();
    this->localPath = query.value("local_path").toString();
    this->type = query.value("type").toInt();
    this->ezpServerUrl = query.value("ezp_server_url").toString();
    this->ezpUsername = query.value("ezp_username").toString();
    this->ezpPassword = query.value("ezp_password").toString();
    this->priority = query.value("priority").toInt();

    return true;
}

QList<LogFileSource> LogFileSource::fetchAll() {
    QSqlDatabase db = QSqlDatabase::database("disk");
    QSqlQuery query(db);

    QList<LogFileSource> logFileSourceList;

    query.prepare("SELECT * FROM logFileSource ORDER BY priority ASC, id ASC");
    if (!query.exec()) {
        qWarning() << __func__ << ": " << query.lastError();
    } else {
        for (int r = 0; query.next(); r++) {
            LogFileSource logFileSource = logFileSourceFromQuery(query);
            logFileSourceList.append(logFileSource);
        }
    }

    return logFileSourceList;
}

/**
 * Inserts or updates a LogFileSource object in the database
 */
bool LogFileSource::store() {
    QSqlDatabase db = QSqlDatabase::database("disk");
    QSqlQuery query(db);

    if (this->id > 0) {
        query.prepare(
                "UPDATE logFileSource SET name = :name, "
                        "type = :type, "
                        "local_path = :localPath, "
                        "ezp_server_url = :ezpServerUrl, "
                        "ezp_username = :ezpUsername, "
                        "ezp_password = :ezpPassword, "
                        "priority = :priority "
                        "WHERE id = :id");
        query.bindValue(":id", this->id);
    } else {
        query.prepare(
                "INSERT INTO logFileSource (name, type, local_path, "
                        "ezp_server_url, ezp_username, ezp_password, "
                        "priority) VALUES (:name, :type, :localPath, "
                        ":ezpServerUrl, :ezpUsername, :ezpPassword, "
                        ":priority)");
    }

    query.bindValue(":name", this->name);
    query.bindValue(":type", this->type);
    query.bindValue(":localPath", this->localPath);
    query.bindValue(":ezpServerUrl", this->ezpServerUrl);
    query.bindValue(":ezpUsername", this->ezpUsername);
    query.bindValue(":ezpPassword", this->ezpPassword);
    query.bindValue(":priority", this->priority);

    if (!query.exec()) {
        // on error
        qWarning() << __func__ << ": " << query.lastError();
        return false;
    } else if (this->id == 0) {
        // on insert
        this->id = query.lastInsertId().toInt();
    }

    return true;
}

/**
 * Checks if the current logFileSource still exists in the database
 */
bool LogFileSource::exists() {
    LogFileSource logFileSource = LogFileSource::fetch(this->id);
    return logFileSource.id > 0;
}

bool LogFileSource::isFetched() {
    return (this->id > 0);
}

void LogFileSource::setAsActive() {
    QSettings settings;
    settings.setValue("activeLogFileSourceId", id);
}

/**
 * Checks if this note folder is the current one
 */
bool LogFileSource::isActive() {
    return activeLogFileSourceId() == id;
}

/**
 * Returns the id of the current note folder in the settings
 */
int LogFileSource::activeLogFileSourceId() {
    QSettings settings;
    return settings.value("activeLogFileSourceId").toInt();
}

/**
 * Returns the current note folder
 */
LogFileSource LogFileSource::activeLogFileSource() {
    return LogFileSource::fetch(activeLogFileSourceId());
}

QDebug operator<<(QDebug dbg, const LogFileSource &logFileSource) {
    dbg.nospace() << "LogFileSource: <id>" << logFileSource.id << " <name>" <<
            logFileSource.name << " <localPath>" << logFileSource.localPath <<
            " <ezpServerUrl>" << logFileSource.ezpServerUrl <<
            " <type>" << logFileSource.type;
    return dbg.space();
}
