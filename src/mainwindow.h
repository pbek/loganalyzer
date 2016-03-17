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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void on_action_Quit_triggered();

    void on_fileListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_removeIgnoredPatternsButton_clicked();

    void on_actionAdd_ignore_pattern_triggered();

    void on_ignoredPatternsListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_ignoredPatternsListWidget_itemChanged(QListWidgetItem *item);

    void on_action_Find_in_file_triggered();

    void on_actionExport_ignore_patterns_triggered();

    void on_actionImport_ignore_patterns_triggered();

    void on_action_Reload_file_triggered();

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

    void findCurrentPattern();

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
};

#endif // MAINWINDOW_H
