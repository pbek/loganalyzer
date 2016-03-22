#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QListWidgetItem>
#include <QSettings>
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

    Q_ENUMS(ToolTabs)

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void on_action_Quit_triggered();

    void on_fileListWidget_currentItemChanged(
            QListWidgetItem *current, QListWidgetItem *previous);

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

protected:
    void closeEvent(QCloseEvent *event);

    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::MainWindow *ui;
    QSplitter *_mainSplitter;
    QSplitter *_leftSplitter;
    QTextEditSearchWidget *_searchWidget;
    QLabel *_lineCountLabel;

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

    void loadLogFile(QFile *file);

    void setupStatusBar();

    void updateLineCount();

    void storeReportPatterns(QSettings *settings = 0);

    void loadReportPatterns(QSettings *settings = 0);

    bool removeSelectedReportPatterns();

    void exportReportPatterns();

    void importReportPatterns();

    void findCurrentReportPattern();
};

#endif // MAINWINDOW_H
