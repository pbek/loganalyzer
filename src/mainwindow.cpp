#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMimeData"
#include <QSettings>
#include <QDropEvent>
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>
#include "QDebug"
#include "QFileDialog"
#include <QDesktopServices>
#include "version.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("LogAnalyzer " + QString(VERSION));

    setupStatusBar();
    ui->fileListWidget->installEventFilter(this);
    ui->ignorePatternsListWidget->installEventFilter(this);
    ui->reportPatternsListWidget->installEventFilter(this);
    ui->fileTextEdit->installEventFilter(this);
    setupMainSplitter();
    setupLeftSplitter();
    readSettings();
    setAcceptDrops(true);
    ui->toolTabWidget->setCurrentIndex(ToolTabs::IgnorePatternTab);
    ui->viewTabWidget->setCurrentIndex(ViewTabs::FileViewTab);

    // add the hidden search widget
    _searchWidget = new QTextEditSearchWidget(ui->fileTextEdit);
    ui->editFrame->layout()->addWidget(_searchWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupStatusBar() {
    _lineCountLabel = new QLabel();
    ui->statusBar->addPermanentWidget(_lineCountLabel);
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
    storeReportPatterns();
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
    loadReportPatterns();
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

            QFileInfo fileInfo = QFileInfo(logFile);

            // skip files that are not readable
            if (fileInfo.isReadable() && fileInfo.isFile()) {
                QListWidgetItem *item = new QListWidgetItem();
                item->setText(logFile);
                ui->fileListWidget->addItem(item);
            }
        }

        ui->fileListWidget->setCurrentRow(0);
    }
}

/**
 * Stores the ignore patterns
 */
void MainWindow::storeIgnorePatterns(QSettings *settings)
{
    QList<QListWidgetItem *> items =
            ui->ignorePatternsListWidget->findItems(
                    QString("*"), Qt::MatchWrap | Qt::MatchWildcard);

    QStringList patternList;
    QList<QVariant> checkedList;

    Q_FOREACH(QListWidgetItem *item, items) {
            patternList.append(item->text());
            checkedList.append(item->checkState() == Qt::Checked);
        }

    if (settings == NULL) {
        settings = new QSettings();
    }

    settings->setValue("ignorePatternExpressions", patternList);
    settings->setValue("ignorePatternCheckedStates", checkedList);
}

/**
 * Loads the ignore patterns
 */
void MainWindow::loadIgnorePatterns(QSettings *settings)
{
    if (settings == NULL) {
        settings = new QSettings();
    }

    QStringList patternList =
            settings->value("ignorePatternExpressions").toStringList();
    QList<QVariant> checkedList =
            settings->value("ignorePatternCheckedStates").toList();

    if (patternList.count() > 0) {
        const QSignalBlocker blocker(this->ui->ignorePatternsListWidget);
        {
            Q_UNUSED(blocker);
            for (int i = 0; i < patternList.count(); i++) {
                QString pattern = patternList.at(i);
                bool checked = checkedList.at(i).toBool();

                QListWidgetItem *item = new QListWidgetItem();
                item->setText(pattern);
                item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
                item->setFlags(item->flags() | Qt::ItemIsEditable);
                ui->ignorePatternsListWidget->addItem(item);
            }
        }
    }
}

/**
 * Stores the report patterns
 */
void MainWindow::storeReportPatterns(QSettings *settings)
{
    QList<QListWidgetItem *> items =
            ui->reportPatternsListWidget->findItems(
                    QString("*"), Qt::MatchWrap | Qt::MatchWildcard);

    QStringList patternList;
    QList<QVariant> checkedList;

    Q_FOREACH(QListWidgetItem *item, items) {
            patternList.append(item->text());
            checkedList.append(item->checkState() == Qt::Checked);
        }

    if (settings == NULL) {
        settings = new QSettings();
    }

    settings->setValue("reportPatternExpressions", patternList);
    settings->setValue("reportPatternCheckedStates", checkedList);
}

/**
 * Loads the report patterns
 */
void MainWindow::loadReportPatterns(QSettings *settings)
{
    if (settings == NULL) {
        settings = new QSettings();
    }

    QStringList patternList =
            settings->value("reportPatternExpressions").toStringList();
    QList<QVariant> checkedList =
            settings->value("reportPatternCheckedStates").toList();

    if (patternList.count() > 0) {
        const QSignalBlocker blocker(this->ui->reportPatternsListWidget);
        {
            Q_UNUSED(blocker);
            for (int i = 0; i < patternList.count(); i++) {
                QString pattern = patternList.at(i);
                bool checked = checkedList.at(i).toBool();

                QListWidgetItem *item = new QListWidgetItem();
                item->setText(pattern);
                item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
                item->setFlags(item->flags() | Qt::ItemIsEditable);
                ui->reportPatternsListWidget->addItem(item);
            }
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

            // check if file is readable
            if (fileInfo.isReadable() && fileInfo.isFile()) {
                // check if we have got the file already
                if (ui->fileListWidget->
                        findItems(path, Qt::MatchExactly).count() == 0) {
                    ui->fileListWidget->addItem(path);
                }
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

        if (obj == ui->ignorePatternsListWidget) {
            if (keyEvent->key() == Qt::Key_Delete) {
                return removeSelectedIgnorePatterns();
            }
        } else if (obj == ui->reportPatternsListWidget) {
            if (keyEvent->key() == Qt::Key_Delete) {
                return removeSelectedReportPatterns();
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
 * Removes all selected ignore patterns from the list
 */
bool MainWindow::removeSelectedIgnorePatterns() {
    int selectedItemsCount =
            ui->ignorePatternsListWidget->selectedItems().size();

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
        qDeleteAll(ui->ignorePatternsListWidget->selectedItems());
        storeIgnorePatterns();
        return true;
    }

    return false;
}

/**
 * Removes all selected report patterns from the list
 */
bool MainWindow::removeSelectedReportPatterns() {
    int selectedItemsCount =
            ui->reportPatternsListWidget->selectedItems().size();

    if (selectedItemsCount == 0) {
        return false;
    }

    if (QMessageBox::information(
                    NULL,
                    tr("Remove selected report patterns"),
                    tr("Remove <strong>%n</strong> report pattern(s)?",
                       "", selectedItemsCount),
                    tr("&Remove"), tr("&Cancel"), QString::null,
                    0, 1) == 0) {
        // remove all selected report patterns
        qDeleteAll(ui->reportPatternsListWidget->selectedItems());
        storeReportPatterns();
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
        // remove all selected log files from the list
        qDeleteAll(ui->fileListWidget->selectedItems());
        storeLogFileList();
        return true;
    }

    return false;
}

/**
 * Finds the currently selected ignore pattern in the currently selected log
 * file
 */
void MainWindow::findCurrentIgnorePattern() {
    QListWidgetItem *item = ui->ignorePatternsListWidget->currentItem();

    if (item == NULL) {
        return;
    }

    qDebug() << __func__ << " - 'item->text()': " << item->text();

    ui->fileTextEdit->moveCursor(QTextCursor::Start);

    QRegExp expression(item->text());
    ui->fileTextEdit->find(expression);
}

/**
 * Finds the currently selected report pattern in the currently selected log
 * file
 */
void MainWindow::findCurrentReportPattern() {
    QListWidgetItem *item = ui->reportPatternsListWidget->currentItem();

    if (item == NULL) {
        return;
    }

    qDebug() << __func__ << " - 'item->text()': " << item->text();

    ui->fileTextEdit->moveCursor(QTextCursor::Start);

    QRegExp expression(item->text());
    ui->fileTextEdit->find(expression);
}

/**
 * Exports the ignore patterns to an ini-file
 */
void MainWindow::exportIgnorePatterns() {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDirectory(QDir::homePath());
    dialog.setNameFilter(tr("INI files (*.ini)"));
    dialog.setWindowTitle(tr("Export ignore patterns as INI"));
    dialog.selectFile("loganalyzer-ignore-patterns.ini");
    int ret = dialog.exec();

    if (ret == QDialog::Accepted) {
        QStringList fileNames = dialog.selectedFiles();
        if (fileNames.count() > 0) {
            QString fileName = fileNames.at(0);

            if (QFileInfo(fileName).suffix().isEmpty()) {
                fileName.append(".ini");
            }

            QSettings *exportSettings =
                    new QSettings(fileName, QSettings::IniFormat);
            storeIgnorePatterns(exportSettings);
        }
    }
}

/**
 * Imports the ignore patterns from ini-files
 */
void MainWindow::importIgnorePatterns() {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDirectory(QDir::homePath());
    dialog.setNameFilter(tr("INI files (*.ini)"));
    dialog.setWindowTitle(tr("Import ignore patterns from INI-file"));
    int ret = dialog.exec();

    if (ret == QDialog::Accepted) {
        QStringList fileNames = dialog.selectedFiles();
        if (fileNames.count() > 0) {
            Q_FOREACH(QString fileName, fileNames) {
                    QSettings *importSettings =
                            new QSettings(fileName, QSettings::IniFormat);
                    loadIgnorePatterns(importSettings);
                }

            ui->toolTabWidget->setCurrentIndex(ToolTabs::IgnorePatternTab);
            storeIgnorePatterns();
        }
    }
}

/**
 * Exports the report patterns to an ini-file
 */
void MainWindow::exportReportPatterns() {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDirectory(QDir::homePath());
    dialog.setNameFilter(tr("INI files (*.ini)"));
    dialog.setWindowTitle(tr("Export report patterns as INI"));
    dialog.selectFile("loganalyzer-report-patterns.ini");
    int ret = dialog.exec();

    if (ret == QDialog::Accepted) {
        QStringList fileNames = dialog.selectedFiles();
        if (fileNames.count() > 0) {
            QString fileName = fileNames.at(0);

            if (QFileInfo(fileName).suffix().isEmpty()) {
                fileName.append(".ini");
            }

            QSettings *exportSettings =
                    new QSettings(fileName, QSettings::IniFormat);
            storeReportPatterns(exportSettings);
        }
    }
}

/**
 * Imports the report patterns from ini-files
 */
void MainWindow::importReportPatterns() {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDirectory(QDir::homePath());
    dialog.setNameFilter(tr("INI files (*.ini)"));
    dialog.setWindowTitle(tr("Import report patterns from INI-file"));
    int ret = dialog.exec();

    if (ret == QDialog::Accepted) {
        QStringList fileNames = dialog.selectedFiles();
        if (fileNames.count() > 0) {
            Q_FOREACH(QString fileName, fileNames) {
                    QSettings *importSettings =
                            new QSettings(fileName, QSettings::IniFormat);
                    loadReportPatterns(importSettings);
                }

            ui->toolTabWidget->setCurrentIndex(ToolTabs::ReportPatternTab);
            storeReportPatterns();
        }
    }
}

/**
 * Updates the line count
 */
void MainWindow::updateLineCount()
{
    // count the lines in the text
    int lineCount = ui->fileTextEdit->toPlainText().count("\n") + 1;

    // set the line count
    _lineCountLabel->setText(tr("%L1 line(s)").arg(lineCount));
}

/**
 * Loads a log file
 */
void MainWindow::loadLogFile(QFile *file)
{
    qDebug() << __func__ << " - 'file': " << file->fileName();

    if (!file->open(QIODevice::ReadOnly)) {
        qDebug() << file->errorString();
        ui->statusBar->showMessage(file->errorString(), 3000);

        return;
    }

    ui->statusBar->showMessage(tr("Loading log file"));

    ui->fileTextEdit->setPlainText(file->readAll());
    file->close();

    // update the line count
    updateLineCount();

    ui->statusBar->showMessage(tr("Done loading log file"), 1000);
}

/**
 * Loads the current log file if a new log file got selected
 */
void MainWindow::on_fileListWidget_currentItemChanged(
        QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);

    if (current == NULL) {
        return;
    }

    QFile file(current->text());

    // load the log file
    loadLogFile(&file);
}

/**
 * Removes occurrences of the ignore patterns from the text
 */
void MainWindow::on_removeIgnoredPatternsButton_clicked()
{
    ui->viewTabWidget->setCurrentIndex(ViewTabs::FileViewTab);

    QList<QListWidgetItem *> items =
            ui->ignorePatternsListWidget->findItems(
                    QString("*"), Qt::MatchWrap | Qt::MatchWildcard);
    QString logText = ui->fileTextEdit->toPlainText();

    ui->statusBar->showMessage(tr("Removing occurrences of the ignore "
                                          "patterns in the text"));

    // remove all occurrences of the ignore patterns from the text
    Q_FOREACH(QListWidgetItem *item, items) {
            if ( item->checkState() != Qt::Checked ) {
                continue;
            }

            QString pattern = item->text();
            qDebug() << __func__ << " - 'pattern': " << pattern;

            QRegularExpression expression(pattern + "\n");
            logText.remove(expression);
        }

    // remove empty lines
    logText = logText.split(QRegExp("\n|\r\n"), QString::SkipEmptyParts)
            .join("\n");

    ui->fileTextEdit->setPlainText(logText);

    // update the line count
    updateLineCount();

    ui->statusBar->showMessage(tr("Done with removing occurrences of the "
                                          "ignore patterns in the text"), 1000);
}

/**
 * Adds an ignore pattern to the list
 */
void MainWindow::on_actionAdd_ignore_pattern_triggered()
{
    ui->toolTabWidget->setCurrentIndex(ToolTabs::IgnorePatternTab);
    QString selectedText = ui->fileTextEdit->textCursor().selectedText();

    QListWidgetItem *item = new QListWidgetItem();
    item->setText(selectedText);
    item->setCheckState(Qt::Checked);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->ignorePatternsListWidget->addItem(item);
    ui->ignorePatternsListWidget->scrollToItem(item);
    ui->ignorePatternsListWidget->editItem(item);

    // store ignore patterns
    storeIgnorePatterns();
}

/**
 * Searches for the current ignore pattern if an other item was selected
 */
void MainWindow::on_ignorePatternsListWidget_currentItemChanged(
        QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(current);
    Q_UNUSED(previous);
    findCurrentIgnorePattern();
}

/**
 * Stores the ignore patterns and searches for the current pattern if an item
 * was changed
 */
void MainWindow::on_ignorePatternsListWidget_itemChanged(QListWidgetItem *item)
{
    // store ignore patterns
    storeIgnorePatterns();

    findCurrentIgnorePattern();
}

void MainWindow::on_action_Find_in_file_triggered()
{
    _searchWidget->activate();
}

void MainWindow::on_actionExport_ignore_patterns_triggered()
{
    exportIgnorePatterns();
}

void MainWindow::on_actionImport_ignore_patterns_triggered()
{
    importIgnorePatterns();
}

/**
 * Reloads the current log file
 */
void MainWindow::on_action_Reload_file_triggered()
{
    QListWidgetItem *item = ui->fileListWidget->currentItem();

    if (item != NULL) {
        QFile file(item->text());

        // load the log file
        loadLogFile(&file);
    }
}

/**
 * Filters ignore patterns by a text
 */
void MainWindow::on_ignorePatternFilterLineEdit_textChanged(const QString &arg1)
{
    // search patterns when at least 2 characters were entered
    if (arg1.count() >= 2) {
        QList<QListWidgetItem*> foundItems = ui->ignorePatternsListWidget->
                findItems(arg1, Qt::MatchContains);

        for (int i = 0; i < this->ui->ignorePatternsListWidget->count(); ++i) {
            QListWidgetItem *item =
                    this->ui->ignorePatternsListWidget->item(i);
            item->setHidden(!foundItems.contains(item));
        }
    } else {
        // show all items otherwise
        for (int i = 0; i < this->ui->ignorePatternsListWidget->count(); ++i) {
            QListWidgetItem *item =
                    this->ui->ignorePatternsListWidget->item(i);
            item->setHidden(false);
        }
    }
}

/**
 * Filters report patterns by a text
 */
void MainWindow::on_reportPatternFilterLineEdit_textChanged(const QString &arg1)
{
    // search patterns when at least 2 characters were entered
    if (arg1.count() >= 2) {
        QList<QListWidgetItem*> foundItems = ui->reportPatternsListWidget->
                findItems(arg1, Qt::MatchContains);

        for (int i = 0; i < this->ui->reportPatternsListWidget->count(); ++i) {
            QListWidgetItem *item =
                    this->ui->reportPatternsListWidget->item(i);
            item->setHidden(!foundItems.contains(item));
        }
    } else {
        // show all items otherwise
        for (int i = 0; i < this->ui->reportPatternsListWidget->count(); ++i) {
            QListWidgetItem *item =
                    this->ui->reportPatternsListWidget->item(i);
            item->setHidden(false);
        }
    }
}

/**
 * Adds an report pattern to the list
 */
void MainWindow::on_actionAdd_report_pattern_triggered()
{
    ui->toolTabWidget->setCurrentIndex(ToolTabs::ReportPatternTab);
    QString selectedText = ui->fileTextEdit->textCursor().selectedText();

    QListWidgetItem *item = new QListWidgetItem();
    item->setText(selectedText);
    item->setCheckState(Qt::Checked);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->reportPatternsListWidget->addItem(item);
    ui->reportPatternsListWidget->scrollToItem(item);
    ui->reportPatternsListWidget->editItem(item);

    // store ignore patterns
    storeIgnorePatterns();
}

void MainWindow::on_actionExport_report_patterns_triggered()
{
    exportReportPatterns();
}

void MainWindow::on_actionImport_report_patterns_triggered()
{
    importReportPatterns();
}

/**
 * Searches for the current report pattern if an other item was selected
 */
void MainWindow::on_reportPatternsListWidget_currentItemChanged(
        QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(current);
    Q_UNUSED(previous);
    findCurrentReportPattern();
}

void MainWindow::on_actionShow_changelog_triggered()
{
    QDesktopServices::openUrl(
            QUrl("https://github.com/pbek/loganalyzer/blob/develop/CHANGELOG.md"));
}

void MainWindow::on_actionReport_issues_triggered()
{
    QDesktopServices::openUrl(
            QUrl("https://github.com/pbek/loganalyzer/issues"));
}

void MainWindow::on_actionShow_releases_triggered()
{
    QDesktopServices::openUrl(
            QUrl("https://github.com/pbek/loganalyzer/releases"));
}

void MainWindow::on_reportPatternsButton_clicked()
{
    ui->viewTabWidget->setCurrentIndex(ViewTabs::ReportViewTab);
}
