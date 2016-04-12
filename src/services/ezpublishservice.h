#pragma once

#include <QNetworkAccessManager>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QObject>
#include "mainwindow.h"


class EzPublishService : public QObject {
Q_OBJECT

public:

    explicit EzPublishService(QObject *parent = 0);

    void loadLogFileList(MainWindow *mainWindow);

private:

    QNetworkAccessManager *networkManager;
    MainWindow *mainWindow;
    static const QString rootPath;
    static const QString format;
    QString logFileListPath;

    void addAuthHeader(QNetworkRequest *r);

    void ignoreSslErrorsIfAllowed(QNetworkReply *reply);

    void showEzPublishServerErrorMessage(
            QString message = QString(""), bool withSettingsButton = true);

signals:

private slots:

    void slotAuthenticationRequired(QNetworkReply *reply,
                                    QAuthenticator *authenticator);

    void slotReplyFinished(QNetworkReply *);
};
