#include "mainwindow.h"
#include "services/ezpublishservice.h"
#include <QDebug>
#include <QUrlQuery>
#include <QMessageBox>
#include <QBuffer>
#include <QJsonDocument>
#include <utils/misc.h>
#include <dialogs/settingsdialog.h>
#include "cryptoservice.h"

const QString EzPublishService::rootPath =
        "/ezjscore/call/loganalyzer::";

EzPublishService::EzPublishService(QObject *parent)
        : QObject(parent) {
    logFileListPath = rootPath + "get_log_file_list";

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

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray arr = reply->readAll();
        QString data = QString(arr);

        if (reply->url().path().endsWith(logFileListPath)) {
            qDebug() << "Reply from log file list";
            qDebug() << data;

            // TODO(pbek): put file names into a string list and output them
            // in the list widget
            qDebug() << QJsonDocument::fromJson(data.toUtf8());

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

