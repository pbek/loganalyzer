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
    ui->ignoredPatternsListWidget->installEventFilter(this);
    setupMainSplitter();
    readSettings();
    setAcceptDrops(true);
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
            "MainWindow/mainSplitterState", mainSplitter->saveState());
    settings.setValue("MainWindow/menuBarGeometry",
                      ui->menuBar->saveGeometry());

    storeIgnorePatterns();
}

void MainWindow::readSettings() {
    QSettings settings;
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    restoreState(settings.value("MainWindow/windowState").toByteArray());
    ui->menuBar->restoreGeometry(
            settings.value("MainWindow/menuBarGeometry").toByteArray());

    loadIgnorePatterns();
}

void MainWindow::setupMainSplitter() {
    mainSplitter = new QSplitter;

    mainSplitter->addWidget(ui->controlFrame);
    mainSplitter->addWidget(ui->editFrame);

    // restore splitter sizes
    QSettings settings;
    QByteArray state =
            settings.value("MainWindow/mainSplitterState").toByteArray();
    mainSplitter->restoreState(state);

    ui->centralWidget->layout()->addWidget(this->mainSplitter);
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
        return true;
    }

    return false;
}

/**
 *
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
    qDebug() << __func__ << " - 'file.isReadable()': " << file.isReadable();

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

            QRegularExpression expression(
                    pattern, QRegularExpression::CaseInsensitiveOption |
                    QRegularExpression::MultilineOption);
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
