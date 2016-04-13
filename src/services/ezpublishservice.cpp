#include "mainwindow.h"
#include "services/ezpublishservice.h"
#include <QDebug>
#include <QUrlQuery>
#include <QMessageBox>
#include <QBuffer>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <utils/misc.h>
#include <dialogs/settingsdialog.h>
#include "cryptoservice.h"

const QString EzPublishService::rootPath =
        "/ezjscore/call/loganalyzer::";

EzPublishService::EzPublishService(QObject *parent)
        : QObject(parent) {
    logFileListPath = rootPath + "get_log_file_list";
    logFileDownloadPath = rootPath + "get_log_file";

    networkManager = new QNetworkAccessManager();

    QObject::connect(networkManager,
                     SIGNAL(authenticationRequired(QNetworkReply * ,
                                                   QAuthenticator *)), this,
                     SLOT(slotAuthenticationRequired(QNetworkReply * ,
                                                     QAuthenticator *)));
    QObject::connect(networkManager, SIGNAL(finished(QNetworkReply *)), this,
                     SLOT(slotReplyFinished(QNetworkReply *)));
}

void EzPublishService::slotAuthenticationRequired(
        QNetworkReply *reply, QAuthenticator *authenticator) {
    Q_UNUSED(authenticator);
    qDebug() << "Username and/or password incorrect";

    reply->abort();
}

void EzPublishService::slotReplyFinished(QNetworkReply *reply) {
    qDebug() << "Reply from " << reply->url().path();
    QByteArray arr = reply->readAll();
    QString data = QString(arr);

    if (reply->error() == QNetworkReply::NoError) {
        if (reply->url().path().endsWith(logFileListPath)) {

            qDebug() << "Reply from log file list";
            qDebug() << data;

            // show the files in the main window
            QJsonArray list =  QJsonDocument::fromJson(data.toUtf8()).array();
            mainWindow->fillEzPublishRemoteFilesListWidget(list);
            return;
        } else if (reply->url().path().endsWith(logFileDownloadPath)) {
            qDebug() << "Reply from log file download";
//            qDebug() << data;

            // TODO(pbek): store local log files
//            LogFileSource logFileSource = LogFileSource::activeLogFileSource();

            QString fileName = getHeaderValue(reply, "X-FILE-NAME");
            mainWindow->updateEzPublishRemoteFileDownloadStatus(fileName, 100);

            return;
        }
    } else {
        if (mainWindow != NULL) {
            showEzPublishServerErrorMessage(reply->errorString());
        }

        qWarning() << "network error: " << reply->errorString();
    }
}

/**
 * Returns a value for a key from the raw header of a QNetworkReply
 */
QString EzPublishService::getHeaderValue(QNetworkReply *reply, QString key) {
    Q_FOREACH(QNetworkReply::RawHeaderPair pair, reply->rawHeaderPairs()) {
            if (key == pair.first) {
                return pair.second;
            }
        }

    return "";
}

/**
 * Ignores ssl errors for a QNetworkReply if allowed
 */
void EzPublishService::ignoreSslErrorsIfAllowed(QNetworkReply *reply) {
    QSettings settings;
    if (settings.value("networking/ignoreSSLErrors", true).toBool()) {
        QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply,
                         SLOT(ignoreSslErrors()));
    }
}

/**
 * Loads the log file list from the active eZ Publish server
 */
void EzPublishService::loadLogFileList(MainWindow *mainWindow) {
    this->mainWindow = mainWindow;

    LogFileSource logFileSource = LogFileSource::activeLogFileSource();
    if (!logFileSource.isEzPublishTypeValid()) {
        showEzPublishServerErrorMessage();
        return;
    }

    QString serverUrl = logFileSource.getEzpServerUrl();
    QUrl url(serverUrl + logFileListPath);

    QNetworkRequest r(url);
    addAuthHeader(&r);

    QNetworkReply *reply = networkManager->get(r);
    ignoreSslErrorsIfAllowed(reply);
}

/**
 * Downloads a the log file from the active eZ Publish server
 */
void EzPublishService::downloadLogFile(MainWindow *mainWindow,
                                       QString fileName) {
    this->mainWindow = mainWindow;

    LogFileSource logFileSource = LogFileSource::activeLogFileSource();
    if (!logFileSource.isEzPublishTypeValid()) {
        showEzPublishServerErrorMessage();
        return;
    }

    QString serverUrl = logFileSource.getEzpServerUrl();
    QUrl url(serverUrl + logFileDownloadPath);

    QUrlQuery q;
    q.addQueryItem("file_name", fileName);
    url.setQuery(q);

    QNetworkRequest r(url);
    addAuthHeader(&r);

    reply = networkManager->get(r);

    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(logFileDownloadProgress(qint64, qint64)));

    ignoreSslErrorsIfAllowed(reply);
}

/**
 * Show percentage of downloaded log file data
 */
void EzPublishService::logFileDownloadProgress(
        qint64 bytesReceived, qint64 bytesTotal) {
    Q_UNUSED(bytesTotal);

    if (reply == NULL) {
        return;
    }

    QString fileName = getHeaderValue(reply, "X-FILE-NAME");
    ulong fileSize = getHeaderValue(reply, "X-FILE-SIZE").toULong();

    double percent = 100 * bytesReceived / fileSize;

    mainWindow->updateEzPublishRemoteFileDownloadStatus(fileName, percent);
}

void EzPublishService::addAuthHeader(QNetworkRequest *r) {
    if (r) {
        LogFileSource logFileSource = LogFileSource::activeLogFileSource();
        if (!logFileSource.isEzPublishTypeValid()) {
            showEzPublishServerErrorMessage();
            return;
        }

        QString userName = logFileSource.getEzpUsername();
        QString password = logFileSource.getEzpPassword();
        QString concatenated = userName + ":" + password;

        QByteArray data = concatenated.toLocal8Bit().toBase64();
        QString headerData = "Basic " + data;
        r->setRawHeader("Authorization", headerData.toLocal8Bit());
    }
}

/**
 * Shows a message dialog with a eZ Publish server error
 */
void EzPublishService::showEzPublishServerErrorMessage(
        QString message, bool withSettingsButton) {
    QString headline = tr("eZ Publish server connection error");
    QString text = message.isEmpty() ?
            "Cannot connect to your eZ Publish server! "
            "Please check your eZ Publish configuration." :
            tr("eZ Publish server error: <strong>%1</strong><br />"
            "Please check your eZ Publish configuration.").arg(message);

    if (withSettingsButton) {
        if (QMessageBox::warning(
                0, headline, text,
                tr("Open &settings"), tr("&Cancel"),
                QString::null, 0, 1) == 0) {
            if (mainWindow != NULL) {
                mainWindow->openSettingsDialog(
                        SettingsDialog::LogFileSourcesTab);
            }
        }
    } else {
        QMessageBox::warning(0, headline, text);
    }
}

