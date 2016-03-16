#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString appNameAdd = "";

#ifdef QT_DEBUG
    appNameAdd = "Debug";
#endif

    QCoreApplication::setOrganizationDomain("PBE");
    QCoreApplication::setOrganizationName("PBE");
    QCoreApplication::setApplicationName("LogAnalyzer" + appNameAdd);

    MainWindow w;
    w.show();

    return a.exec();
}
