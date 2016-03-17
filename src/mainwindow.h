#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QListWidgetItem>
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

    void on_removeIgnoredPattersButton_clicked();

    void on_actionAdd_ignore_pattern_triggered();

    void on_ignoredPatternsListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_ignoredPatternsListWidget_itemChanged(QListWidgetItem *item);

    void on_action_Find_in_file_triggered();

protected:
    void closeEvent(QCloseEvent *event);

    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::MainWindow *ui;
    QSplitter *_mainSplitter;
    QSplitter *_leftSplitter;
    QTextEditSearchWidget *_searchWidget;

    void setupMainSplitter();

    void storeSettings();

    void readSettings();

    bool removeSelectedIgnorePatterns();

    void findCurrentPattern();

    void storeIgnorePatterns();

    void loadIgnorePatterns();

    bool removeLogFiles();

    void storeLogFileList();

    void loadLogFileList();

    void initSearchFrame();

    void setupLeftSplitter();
};

#endif // MAINWINDOW_H
