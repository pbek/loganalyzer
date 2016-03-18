#include "mainwindow.h"
#include "version.h"
#include "release.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // fixing some troubles in Windows 8.1
#ifdef Q_OS_WIN32
    QCoreApplication::addLibraryPath("./");
#endif

    QApplication a(argc, argv);
    QString appNameAdd = "";

#ifdef QT_DEBUG
    appNameAdd = "Debug";
#endif

    QCoreApplication::setOrganizationDomain("PBE");
    QCoreApplication::setOrganizationName("PBE");
    QCoreApplication::setApplicationName("LogAnalyzer" + appNameAdd);

    QString appVersion = QString(VERSION);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    appVersion += " " + QSysInfo::prettyProductName();

    if (!appVersion.contains(QSysInfo::currentCpuArchitecture())) {
        appVersion += " " + QSysInfo::currentCpuArchitecture();
    }
#else
    appVersion += " Qt " + QString(QT_VERSION_STR);
#endif

    appVersion += " " + QString(RELEASE);

#ifdef QT_DEBUG
    appVersion += " Debug";
#endif

    QCoreApplication::setApplicationVersion(appVersion);

    MainWindow w;
    w.show();

    return a.exec();
}
