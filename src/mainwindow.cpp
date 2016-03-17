#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMimeData"
#include <QSettings>
#include <QDropEvent>
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>
#include "QDebug"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->fileListWidget->installEventFilter(this);
    ui->ignoredPatternsListWidget->installEventFilter(this);
    ui->fileTextEdit->installEventFilter(this);
    setupMainSplitter();
    setupLeftSplitter();
    readSettings();
    setAcceptDrops(true);

    // add the hidden search widget
    _searchWidget = new QTextEditSearchWidget(ui->fileTextEdit);
    ui->editFrame->layout()->addWidget(_searchWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::storeSettings() {
    QSettings settings;

    settings.setValue("MainWindow/geometry", saveGeometry());
    settings.setValue("MainWindow/windowState", saveState());
    settings.setValue(
            "MainWindow/mainSplitterState", _mainSplitter->saveState());
    settings.setValue(
            "MainWindow/leftSplitterState", _leftSplitter->saveState());
    settings.setValue("MainWindow/menuBarGeometry",
                      ui->menuBar->saveGeometry());

    storeIgnorePatterns();
    storeLogFileList();
}

void MainWindow::readSettings() {
    QSettings settings;
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    restoreState(settings.value("MainWindow/windowState").toByteArray());
    ui->menuBar->restoreGeometry(
            settings.value("MainWindow/menuBarGeometry").toByteArray());

    loadLogFileList();
    loadIgnorePatterns();
}

void MainWindow::setupMainSplitter() {
    _mainSplitter = new QSplitter;

    _mainSplitter->addWidget(ui->controlFrame);
    _mainSplitter->addWidget(ui->editFrame);

    // restore splitter sizes
    QSettings settings;
    QByteArray state =
            settings.value("MainWindow/mainSplitterState").toByteArray();
    _mainSplitter->restoreState(state);

    ui->centralWidget->layout()->addWidget(this->_mainSplitter);
}

void MainWindow::setupLeftSplitter() {
    _leftSplitter = new QSplitter(Qt::Vertical);

    _leftSplitter->addWidget(ui->fileListWidget);
    _leftSplitter->addWidget(ui->toolTabWidget);

    // restore splitter sizes
    QSettings settings;
    QByteArray state =
            settings.value("MainWindow/leftSplitterState").toByteArray();
    _leftSplitter->restoreState(state);

    ui->controlFrame->layout()->addWidget(this->_leftSplitter);
}

/**
 * Stores the log file list
 */
void MainWindow::storeLogFileList()
{
    QList<QListWidgetItem *> items =
            ui->fileListWidget->findItems(
                    QString("*"), Qt::MatchWrap | Qt::MatchWildcard);

    QStringList logFiles;

    Q_FOREACH(QListWidgetItem *item, items) {
            logFiles.append(item->text());
        }

    QSettings settings;
    settings.setValue("logFiles", logFiles);
}

/**
 * Loads the log file list
 */
void MainWindow::loadLogFileList()
{
    QSettings settings;
    QStringList logFiles = settings.value("logFiles").toStringList();

    if (logFiles.count() > 0) {
        ui->fileListWidget->clear();

        for (int i = 0; i < logFiles.count(); i++) {
            QString logFile = logFiles.at(i);

            QListWidgetItem *item = new QListWidgetItem();
            item->setText(logFile);
            ui->fileListWidget->addItem(item);
        }

        ui->fileListWidget->setCurrentRow(0);
    }
}

/**
 * Stores the ignore patterns
 */
void MainWindow::storeIgnorePatterns()
{
    QList<QListWidgetItem *> items =
            ui->ignoredPatternsListWidget->findItems(
                    QString("*"), Qt::MatchWrap | Qt::MatchWildcard);

    QStringList patternList;
    QList<QVariant> checkedList;

    Q_FOREACH(QListWidgetItem *item, items) {
            patternList.append(item->text());
            checkedList.append(item->checkState() == Qt::Checked);
        }

    QSettings settings;
    settings.setValue("ignorePatternExpressions", patternList);
    settings.setValue("ignorePatternCheckedStates", checkedList);
}

/**
 * Loads the ignore patterns
 */
void MainWindow::loadIgnorePatterns()
{
    QSettings settings;
    QStringList patternList =
            settings.value("ignorePatternExpressions").toStringList();
    QList<QVariant> checkedList =
            settings.value("ignorePatternCheckedStates").toList();

    if (patternList.count() > 0) {
        ui->ignoredPatternsListWidget->clear();

        for (int i = 0; i < patternList.count(); i++) {
            QString pattern = patternList.at(i);
            bool checked = checkedList.at(i).toBool();

            QListWidgetItem *item = new QListWidgetItem();
            item->setText(pattern);
            item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            ui->ignoredPatternsListWidget->addItem(item);
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

/**
 * Handles the copying of notes to the current notes folder
 */
void MainWindow::dropEvent(QDropEvent *e) {
    const QMimeData *mimeData = e->mimeData();

    if (mimeData->hasUrls()) {
        foreach(const QUrl &url, mimeData->urls()) {
            QString path(url.toLocalFile());
            QFileInfo fileInfo(path);

            if (fileInfo.isReadable() && fileInfo.isFile()) {
                ui->fileListWidget->addItem(path);
            }
        }

        storeLogFileList();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    storeSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::on_action_Quit_triggered()
{
    storeSettings();
    QApplication::quit();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (obj == ui->ignoredPatternsListWidget) {
            if (keyEvent->key() == Qt::Key_Delete) {
                return removeIgnorePatterns();
            }
        } else if (obj == ui->fileListWidget) {
            if (keyEvent->key() == Qt::Key_Delete) {
                return removeLogFiles();
            }
        } else if (obj == ui->fileTextEdit) {
            if ((keyEvent->key() == Qt::Key_Escape) &&
                    _searchWidget->isVisible()) {
                _searchWidget->deactivate();
                return true;
            } else if ((keyEvent->key() == Qt::Key_F3)) {
                _searchWidget->doSearch(
                        !keyEvent->modifiers().testFlag(Qt::ShiftModifier));
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

/**
 * Removes all selected ignore patterns
 */
bool MainWindow::removeIgnorePatterns() {
    int selectedItemsCount =
            ui->ignoredPatternsListWidget->selectedItems().size();

    if (selectedItemsCount == 0) {
        return false;
    }

    if (QMessageBox::information(
                    NULL,
                    tr("Remove selected ignore patterns"),
                    tr("Remove <strong>%n</strong> ignore pattern(s)?",
                       "", selectedItemsCount),
                    tr("&Remove"), tr("&Cancel"), QString::null,
                    0, 1) == 0) {
        // remove all selected ignore patterns
        qDeleteAll(ui->ignoredPatternsListWidget->selectedItems());
        storeIgnorePatterns();
        return true;
    }

    return false;
}

/**
 * Removes all selected log files
 */
bool MainWindow::removeLogFiles() {
    int selectedItemsCount =
            ui->fileListWidget->selectedItems().size();

    if (selectedItemsCount == 0) {
        return false;
    }

    if (QMessageBox::information(
                    NULL,
                    tr("Remove selected files"),
                    tr("Remove <strong>%n</strong> file(s) from list?",
                       "", selectedItemsCount),
                    tr("&Remove"), tr("&Cancel"), QString::null,
                    0, 1) == 0) {
        // remove all selected ignore patterns
        qDeleteAll(ui->fileListWidget->selectedItems());
        storeLogFileList();
        return true;
    }

    return false;
}

/**
 * Uses an other widget as parent for the search widget
 */
void MainWindow::initSearchFrame() {
    // remove the search widget from our layout
    layout()->removeWidget(ui->editFrame);

    QLayout *layout = ui->editFrame->layout();
    layout->addWidget(ui->editFrame);
    ui->editFrame->setLayout(layout);
}

/**
 * Finds the currently selected pattern in the currently selected log file
 */
void MainWindow::findCurrentPattern() {
    QListWidgetItem *item = ui->ignoredPatternsListWidget->currentItem();

    if (item == NULL) {
        return;
    }

    qDebug() << __func__ << " - 'item->text()': " << item->text();

    ui->fileTextEdit->moveCursor(QTextCursor::Start);

    QRegExp expression(item->text());
    ui->fileTextEdit->find(expression);
}

/**
 * Loads a log file
 */
void MainWindow::on_fileListWidget_currentItemChanged(
        QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);

    if (current == NULL) {
        return;
    }

    QFile file(current->text());
    qDebug() << __func__ << " - 'file': " << file.fileName();

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString();
        return;
    }

    ui->fileTextEdit->setPlainText(file.readAll());
    file.close();
}

void MainWindow::on_removeIgnoredPattersButton_clicked()
{
    QList<QListWidgetItem *> items =
            ui->ignoredPatternsListWidget->findItems(
                    QString("*"), Qt::MatchWrap | Qt::MatchWildcard);
    QString logText = ui->fileTextEdit->toPlainText();

    Q_FOREACH(QListWidgetItem *item, items) {
            if ( item->checkState() != Qt::Checked ) {
                continue;
            }

            QString pattern = item->text();
            qDebug() << __func__ << " - 'pattern': " << pattern;

            QRegularExpression expression(pattern + "\n");
//            QRegExp expression(pattern);
            logText.remove(expression);
        }

    ui->fileTextEdit->setPlainText(logText);
}

void MainWindow::on_actionAdd_ignore_pattern_triggered()
{
    QString selectedText = ui->fileTextEdit->textCursor().selectedText();

    QListWidgetItem *item = new QListWidgetItem();
    item->setText(selectedText);
    item->setCheckState(Qt::Checked);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->ignoredPatternsListWidget->addItem(item);

    // store ignore patterns
    storeIgnorePatterns();
}

void MainWindow::on_ignoredPatternsListWidget_currentItemChanged(
        QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(current);
    Q_UNUSED(previous);
    findCurrentPattern();
}

void MainWindow::on_ignoredPatternsListWidget_itemChanged(QListWidgetItem *item)
{
    // store ignore patterns
    storeIgnorePatterns();

    findCurrentPattern();
}

void MainWindow::on_action_Find_in_file_triggered()
{
    _searchWidget->activate();
}
