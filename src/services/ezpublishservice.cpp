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
#include <QDir>
#include <utils/misc.h>
#include <dialogs/settingsdialog.h>
#include "cryptoservice.h"

const QString EzPublishService::rootPath =
        "/ezjscore/call/loganalyzer::";

EzPublishService::EzPublishService(QObject *parent)
        : QObject(parent) {
    logFileListPath = rootPath + "get_log_file_list";
    logFileDownloadPath = rootPath + "get_log_file";
    connectionTestPath = rootPath + "connection_test";

    networkManager = new QNetworkAccessManager();

    QObject::connect(networkManager,
                     SIGNAL(authenticationRequired(QNetworkReply * ,
                                                   QAuthenticator *)), this,
                     SLOT(slotAuthenticationRequired(QNetworkReply * ,
                                                     QAuthenticator *)));
    QObject::connect(networkManager, SIGNAL(finished(QNetworkReply *)), this,
                     SLOT(slotReplyFinished(QNetworkReply *)));

    _logFileSource = LogFileSource::activeLogFileSource();
}

void EzPublishService::slotAuthenticationRequired(
        QNetworkReply *reply, QAuthenticator *authenticator) {
    Q_UNUSED(authenticator);

    if (reply == NULL) {
        return;
    }

    reply->abort();
}

void EzPublishService::slotReplyFinished(QNetworkReply *reply) {
    if (reply == NULL) {
        return;
    }

    qDebug() << "Reply from " << reply->url().path();
    QByteArray arr = reply->readAll();
    QString data = QString(arr);

    // handle requests for the connection test page
    if (reply->url().path().endsWith(connectionTestPath)) {
        qDebug() << "Reply from connection test";

        if (reply->error() == QNetworkReply::NoError) {
            if (data == "successfully connected") {
                // output that everything is all right
                _settingsDialog->setConnectionTestMessage(
                        tr("A connection to the server was "
                                   "made successfully!"));
            } else if (data.startsWith(
                    "Not a valid ezjscServerRouter argument")) {
                // output the error message from the server if the
                // connection test url wasn't found
                _settingsDialog->setConnectionTestMessage(
                        tr("An error occurred, it seems that the 'loganalyzer'"
                                   " extension isn't installed correctly:\n%1")
                                .arg(data), true);
            } else {
                // output the error message from the server
                _settingsDialog->setConnectionTestMessage(
                        tr("An error occurred:\n%1").arg(data), true);
            }
        } else {
            // output a network connection error
            _settingsDialog->setConnectionTestMessage(
                    tr("An error occurred:\n%1").arg(reply->errorString()),
                    true);
        }

        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        if (reply->url().path().endsWith(logFileListPath)) {
            qDebug() << "Reply from log file list";
//            qDebug() << data;

            // show the files in the main window
            QJsonArray list = QJsonDocument::fromJson(data.toUtf8()).array();
            mainWindow->fillEzPublishRemoteFilesListWidget(list);
            return;
        } else if (reply->url().path().endsWith(logFileDownloadPath)) {
            qDebug() << "Reply from log file download";
//            qDebug() << data;

            QString fileName = getHeaderValue(reply, "X-FILE-NAME");
            mainWindow->updateEzPublishRemoteFileDownloadStatus(fileName, 100);

            // decompress log file if it was compressed
            if (fileName.endsWith(".gz")) {
                try {
                    data = QString(Utils::Misc::gUncompress(arr));
                } catch(std::exception const& e) {
                    QMessageBox::critical(
                            0, tr("Could not decompress file"),
                            tr("Could not decompress file:\n%1").arg(e.what()));
                    return;
                }

                fileName = Utils::Misc::removeIfEndsWith(fileName, ".gz");
            }

            // use a prefix if we have to
            QString prefix = _logFileSource.getAddDownloadedFilePrefix() ?
                             _logFileSource.getName() + " - " : "";

            // generate local log file path
            QString localFilePath =
                    _logFileSource.getLocalPath() + QDir::separator() +
                            prefix + fileName;

            // choose a proper suffix if the file already exists
            localFilePath = chooseFileNameSuffix(localFilePath);

            qDebug() << __func__ << " - 'localFilePath': " << localFilePath;

            QFile file(localFilePath);

            // store local log file
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << data;
                file.flush();
                file.close();
            } else {
                QMessageBox::critical(
                        0, tr("Could not store to file"),
                        tr("Could not store to file:\n%1\n\n%2\n\nIs your "
                                   "local path writable?")
                                .arg(localFilePath, file.errorString()));
            }

            return;
        }
    } else {
        if (mainWindow != NULL) {
            showEzPublishServerErrorMessage(reply->errorString());
        }

        qWarning() << tr("network error: %1").arg(reply->errorString());
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

    if (!_logFileSource.isEzPublishTypeValid()) {
        showEzPublishServerErrorMessage();
        return;
    }

    QString serverUrl = _logFileSource.getEzpServerUrl();
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

    if (!_logFileSource.isEzPublishTypeValid()) {
        showEzPublishServerErrorMessage();
        return;
    }

    QString serverUrl = _logFileSource.getEzpServerUrl();
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

    double percent = fileSize > 0 ? 100 * bytesReceived / fileSize : 0;

    mainWindow->updateEzPublishRemoteFileDownloadStatus(fileName, percent);
}

void EzPublishService::addAuthHeader(QNetworkRequest *r) {
    if (r) {
        if (!_logFileSource.isEzPublishTypeValid()) {
            showEzPublishServerErrorMessage();
            return;
        }

        QString userName = _logFileSource.getEzpUsername();
        QString password = _logFileSource.getEzpPassword();
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
        if (QMessageBox::critical(
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

/**
 * Returns a filename with a proper suffix if the file already exists
 */
QString EzPublishService::chooseFileNameSuffix(QString filePath, int suffix) {
    QString newFilePath = filePath;

    if (suffix > 0) {
        // we are choose an unusual suffix here because the log files will
        // already be named like `logfile.log.1`, `logfile.log.2` and so on
        newFilePath += ".v" + QString::number(suffix);
    }

    QFileInfo file(newFilePath);

    if (file.exists()) {
        return chooseFileNameSuffix(filePath, ++suffix);
    } else {
        return newFilePath;
    }
}

/**
 * Starts a connection test for the settings
 */
void EzPublishService::settingsConnectionTest(SettingsDialog *dialog,
                                              LogFileSource logFileSource) {
    if (!logFileSource.isEzPublishTypeValid()) {
        dialog->setConnectionTestMessage(
                tr("Please complete your eZ Publish settings"), true);
        return;
    }

    _settingsDialog = dialog;
    _logFileSource = logFileSource;

    qDebug() << __func__ << " - 'logFileSource': " << logFileSource;

    QString serverUrl = logFileSource.getEzpServerUrl();
    QUrl url(serverUrl + connectionTestPath);

    QNetworkRequest r(url);
    addAuthHeader(&r);

    QNetworkReply *reply = networkManager->get(r);
    ignoreSslErrorsIfAllowed(reply);
}
