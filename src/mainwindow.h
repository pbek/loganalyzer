#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QListWidgetItem>
#include <QSettings>
#include <QFileSystemWatcher>
#include <QJsonArray>
#include <QTableWidgetItem>
#include <entities/logfilesource.h>
#include "qtexteditsearchwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum ToolTabs {
        IgnorePatternTab,
        ReportPatternTab
    };

    enum ViewTabs {
        FileViewTab,
        ReportViewTab
    };

    Q_ENUMS(ToolTabs)

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void openSettingsDialog(int tab = 0);

    void fillEzPublishRemoteFilesListWidget(QJsonArray fileList);

private slots:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void on_action_Quit_triggered();

    void on_removeIgnoredPatternsButton_clicked();

    void on_actionAdd_ignore_pattern_triggered();

    void on_ignorePatternsListWidget_currentItemChanged(
            QListWidgetItem *current, QListWidgetItem *previous);

    void on_ignorePatternsListWidget_itemChanged(QListWidgetItem *item);

    void on_action_Find_in_file_triggered();

    void on_actionExport_ignore_patterns_triggered();

    void on_actionImport_ignore_patterns_triggered();

    void on_action_Reload_file_triggered();

    void on_ignorePatternFilterLineEdit_textChanged(const QString &arg1);

    void on_actionAdd_report_pattern_triggered();

    void on_actionExport_report_patterns_triggered();

    void on_actionImport_report_patterns_triggered();

    void on_reportPatternsListWidget_currentItemChanged(
            QListWidgetItem *current, QListWidgetItem *previous);

    void on_reportPatternFilterLineEdit_textChanged(const QString &arg1);

    void on_actionShow_changelog_triggered();

    void on_actionReport_issues_triggered();

    void on_actionShow_releases_triggered();

    void on_reportPatternsButton_clicked();

    void on_viewTabWidget_tabBarClicked(int index);

    void on_action_Export_report_as_PDF_triggered();

    void on_action_Print_report_triggered();

    void on_fileListWidget_itemSelectionChanged();

    void on_ignorePatternFilterLineEdit_returnPressed();

    void on_reportPatternFilterLineEdit_returnPressed();

    void on_action_Settings_triggered();

    void on_logFileSourceComboBox_currentIndexChanged(int index);

    void on_localFileUsePushButton_clicked();

    void on_localFilesListWidget_itemDoubleClicked(QListWidgetItem *item);

    void loadLocalLogFileSourceFiles(QString localPath);

    void on_logFileSourceRemoteReloadButton_clicked();

    void on_logFileSourceRemoteDownloadButton_clicked();

protected:
    void closeEvent(QCloseEvent *event);

    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::MainWindow *ui;
    QSplitter *_mainSplitter;
    QSplitter *_leftSplitter;
    QTextEditSearchWidget *_searchWidget;
    QLabel *_lineCountLabel;
    QFileSystemWatcher *_localDirectoryWatcher;

    void setupMainSplitter();

    void storeSettings();

    void readSettings();

    bool removeSelectedIgnorePatterns();

    void findCurrentIgnorePattern();

    void storeIgnorePatterns(QSettings *settings = 0);

    void loadIgnorePatterns(QSettings *settings = 0);

    bool removeLogFiles();

    void storeLogFileList();

    void loadLogFileList();

    void setupLeftSplitter();

    void exportIgnorePatterns();

    void importIgnorePatterns();

    void loadLogFiles();

    void setupStatusBar();

    void updateLineCount();

    void storeReportPatterns(QSettings *settings = 0);

    void loadReportPatterns(QSettings *settings = 0);

    bool removeSelectedReportPatterns();

    void exportReportPatterns();

    void importReportPatterns();

    void findCurrentReportPattern();

    void exportTextEditContentAsPDF(QTextEdit *textEdit);

    void printTextEditContent(QTextEdit *textEdit);

    void addReportPattern(QString text);

    void addIgnorePattern(QString text);

    void readSettingsFromSettingsDialog();

    void updateLogFileSourceComboBox();

    void changeLogFileSource(LogFileSource logFileSource);

    void addPathToFileListWidget(QString path);

    void setLocalDirectoryWatcherPath(QString path);
};


class FileSizeTableWidgetItem : public QTableWidgetItem {
public:
    bool operator <(const QTableWidgetItem &other) const
    {
        return data(Qt::UserRole).toInt() < other.data(Qt::UserRole).toInt();
    }
};

#endif // MAINWINDOW_H
