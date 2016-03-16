#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QListWidgetItem>

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

protected:
    void closeEvent(QCloseEvent *event);

    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::MainWindow *ui;
    QSplitter *mainSplitter;

    void setupMainSplitter();

    void storeSettings();

    void readSettings();

    bool removeIgnorePatterns();

    void findCurrentPattern();

    void storeIgnorePatterns();

    void loadIgnorePatterns();
};

#endif // MAINWINDOW_H
