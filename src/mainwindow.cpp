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
#include <QPrinter>
#include <QPrintDialog>
#include "version.h"
#include <dialogs/settingsdialog.h>
#include <services/databaseservice.h>
#include <services/ezpublishservice.h>
#include <QtCore/QJsonObject>
#include <utils/misc.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("LogAnalyzer " + QString(VERSION));

    DatabaseService::createConnection();
    DatabaseService::setupTables();

    _localDirectoryWatcher = new QFileSystemWatcher(this);
    QObject::connect(
            _localDirectoryWatcher,
            SIGNAL(directoryChanged(QString)),
            this,
            SLOT(loadLocalLogFileSourceFiles(QString)));

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

    // read all relevant settings, that can be set in the settings dialog
    readSettingsFromSettingsDialog();
}

void MainWindow::setupMainSplitter() {
    _mainSplitter = new QSplitter;

    _mainSplitter->addWidget(ui->logFileSourceFrame);
    _mainSplitter->addWidget(ui->controlFrame);
    _mainSplitter->addWidget(ui->editFrame);

    QSettings settings;
    int reset = settings.value("MainWindow/mainSplitterReset").toInt();

    // we need to reset the splitter once for the logFileSourceFrame
    if (reset > 0) {
        // restore splitter sizes
        QByteArray state =
                settings.value("MainWindow/mainSplitterState").toByteArray();
        _mainSplitter->restoreState(state);
    } else {
        settings.setValue("MainWindow/mainSplitterReset", 1);
    }

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
                addPathToFileListWidget(logFile);
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
                addPathToFileListWidget(path);
            }
        }

        storeLogFileList();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    storeSettings();
    QMainWindow::closeEvent(event);
}

/**
 * Quits the application
 */
void MainWindow::on_action_Quit_triggered()
{
    storeSettings();
    QApplication::quit();
}

/**
 * Handles the event filtering
 */
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
        const QSignalBlocker blocker(this->ui->fileListWidget);
        Q_UNUSED(blocker);

        // remove all selected log files from the list
        qDeleteAll(ui->fileListWidget->selectedItems());
        storeLogFileList();

        ui->fileTextEdit->clear();

        // update the line count
        updateLineCount();

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
void MainWindow::loadLogFiles()
{
    QString logString;

    QList<QListWidgetItem *> items = ui->fileListWidget->selectedItems();
    Q_FOREACH(QListWidgetItem *item, items) {
            qDebug() << __func__ << " - 'item': " << item;

            QFile file(item->text());

            if (!file.exists()) {
                QString warning = tr("File '%1' does not exist!")
                                      .arg(file.fileName());
                qWarning() << warning;
                ui->statusBar->showMessage(warning);
                continue;
            }

            if (!file.open(QIODevice::ReadOnly)) {
                qWarning() << file.errorString();
                ui->statusBar->showMessage(file.errorString());
                continue;
            }

            qDebug() << __func__ << " - 'file': " << file.fileName();

            ui->statusBar->showMessage(
                    tr("Loading log file <strong>%1</strong>")
                            .arg(file.fileName()));

            logString += file.readAll();
            file.close();
        }

    ui->statusBar->showMessage(tr("Updating line count"));

    ui->fileTextEdit->setPlainText(logString);

    // update the line count
    updateLineCount();

    ui->statusBar->showMessage(tr("Done loading log files"), 1000);
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

    // add an ignore pattern
    addIgnorePattern(selectedText);
}

/**
 * Adds text as ignore pattern to the list
 */
void MainWindow::addIgnorePattern(QString text)
{
    QListWidgetItem *item = new QListWidgetItem();
    item->setText(text);
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
    Q_UNUSED(item);

    // store ignore patterns
    storeIgnorePatterns();

    findCurrentIgnorePattern();
}

/**
 * Opens the find widget
 */
void MainWindow::on_action_Find_in_file_triggered()
{
    _searchWidget->activate();
}

/**
 * Exports ignore patterns
 */
void MainWindow::on_actionExport_ignore_patterns_triggered()
{
    exportIgnorePatterns();
}

/**
 * Imports ignore patterns
 */
void MainWindow::on_actionImport_ignore_patterns_triggered()
{
    importIgnorePatterns();
}

/**
 * Reloads the current log file
 */
void MainWindow::on_action_Reload_file_triggered()
{
    // load the log files
    loadLogFiles();
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
 * Adds a report pattern to the list
 */
void MainWindow::on_actionAdd_report_pattern_triggered()
{
    ui->toolTabWidget->setCurrentIndex(ToolTabs::ReportPatternTab);
    QString selectedText = ui->fileTextEdit->textCursor().selectedText();

    // add report pattern
    addReportPattern(selectedText);
}

/**
 * Adds text as report pattern to the list
 */
void MainWindow::addReportPattern(QString text)
{
    QListWidgetItem *item = new QListWidgetItem();
    item->setText(text);
    item->setCheckState(Qt::Checked);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->reportPatternsListWidget->addItem(item);
    ui->reportPatternsListWidget->scrollToItem(item);
    ui->reportPatternsListWidget->editItem(item);

    // store ignore patterns
    storeIgnorePatterns();
}

/**
 * Exports report patterns
 */
void MainWindow::on_actionExport_report_patterns_triggered()
{
    exportReportPatterns();
}

/**
 * Imports report patterns
 */
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

/**
 * Opens the change log on GitHub
 */
void MainWindow::on_actionShow_changelog_triggered()
{
    QDesktopServices::openUrl(
            QUrl("https://github.com/pbek/loganalyzer/blob/develop/"
                         "CHANGELOG.md"));
}

/**
 * Opens the issues page on GitHub
 */
void MainWindow::on_actionReport_issues_triggered()
{
    QDesktopServices::openUrl(
            QUrl("https://github.com/pbek/loganalyzer/issues"));
}

/**
 * Opens the releases page on GitHub
 */
void MainWindow::on_actionShow_releases_triggered()
{
    QDesktopServices::openUrl(
            QUrl("https://github.com/pbek/loganalyzer/releases"));
}

/**
 * Generates a report from the report patterns
 */
void MainWindow::on_reportPatternsButton_clicked()
{
    ui->viewTabWidget->setCurrentIndex(ViewTabs::ReportViewTab);

    QList<QListWidgetItem *> items =
            ui->reportPatternsListWidget->findItems(
                    QString("*"), Qt::MatchWrap | Qt::MatchWildcard);
    QString logText = ui->fileTextEdit->toPlainText();
    QString reportHtml = "<html>"
            "<head>"
            "<style>"
            "* {font-family: 'Open Sans', Arial, Helvetica, sans-serif;}"
            "pre, code, h2.code {"
            "font-family: 'Droid Sans Mono', monospace;"
            "}"
            "pre, code {"
            "white-space: pre-wrap;"
            "background-color: #efefef;"
            "}"
            "h2 {margin: 30px 0 20px 0;}"
            "</style>"
            "<head>"
            "<body>";
    reportHtml += tr("<h1>LogAnalyzer report</h1>");

    // add log file information
    QList<QListWidgetItem *> logFileItems = ui->fileListWidget->selectedItems();
    if (logFileItems.count() > 0) {
        reportHtml += tr("<h2>Log files</h2><ul>");

        Q_FOREACH(QListWidgetItem *logFileItem, logFileItems) {
                reportHtml += QString("<li>%1</li>").arg(logFileItem->text());
            }

        reportHtml += tr("</ul>");
    }

    ui->statusBar->showMessage(tr("Reporting on occurrences of the report "
                                          "patterns in the text"));

    // find all occurrences of the report patterns from the text
    for (int index = 0; index < items.count(); index++) {
        QListWidgetItem *item = items.at(index);

        if ( item->checkState() != Qt::Checked ) {
            continue;
        }

        QString pattern = item->text();
        QHash<QString, int> matchesCounts;

        QRegularExpression re(pattern);
        QRegularExpressionMatchIterator iterator = re.globalMatch(logText);
        while (iterator.hasNext()) {
            QRegularExpressionMatch match = iterator.next();
            QString text = match.captured(1);

            if (text.isEmpty()) {
                text = match.captured(0);
            }

            matchesCounts[text]++;
        }


        if (matchesCounts.count() > 0) {
            reportHtml += QString("<h2 class='code'>%1</h2><ul>").arg(pattern);

            QHashIterator<QString, int> i(matchesCounts);
            while (i.hasNext()) {
                i.next();
                QString text = i.key();
                int count = i.value();

                reportHtml +=
                        "<li>"
                        + tr("<pre>%1</pre> found: %n time(s)", "", count)
                                                          .arg(text)
                        + "</li>";
            }

            reportHtml += "</ul>";
        }
    }

    reportHtml += "</body></html>";

    ui->reportTextEdit->setHtml(reportHtml);
    ui->statusBar->showMessage(tr("Done with reporting occurrences of the "
                                          "report patterns in the text"), 1000);
}

/**
 * Generates the report if the report view tab was clicked
 */
void MainWindow::on_viewTabWidget_tabBarClicked(int index)
{
    if (index == ViewTabs::ReportViewTab) {
        on_reportPatternsButton_clicked();
    }
}

/**
 * Exports the report as PDF
 */
void MainWindow::on_action_Export_report_as_PDF_triggered()
{
    on_reportPatternsButton_clicked();
    exportTextEditContentAsPDF(ui->reportTextEdit);
}

/**
 * Prints the report
 */
void MainWindow::on_action_Print_report_triggered()
{
    on_reportPatternsButton_clicked();
    printTextEditContent(ui->reportTextEdit);
}


/**
 * Prints the content of a text edit widget
 * @param textEdit
 */
void MainWindow::printTextEditContent(QTextEdit *textEdit) {
    QPrinter printer;

    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle(tr("Print report"));

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    textEdit->document()->print(&printer);
}

/**
 * Exports the content of a text edit widget as PDF
 * @param textEdit
 */
void MainWindow::exportTextEditContentAsPDF(QTextEdit *textEdit) {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDirectory(QDir::homePath());
    dialog.setNameFilter(tr("PDF files (*.pdf)"));
    dialog.setWindowTitle(tr("Export report as PDF"));
    dialog.selectFile("LogAnalyzer Report.pdf");
    int ret = dialog.exec();

    if (ret == QDialog::Accepted) {
        QStringList fileNames = dialog.selectedFiles();
        if (fileNames.count() > 0) {
            QString fileName = fileNames.at(0);

            if (QFileInfo(fileName).suffix().isEmpty()) {
                fileName.append(".pdf");
            }

            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(fileName);
            textEdit->document()->print(&printer);
        }
    }
}

/**
 * Loads the selected log files
 */
void MainWindow::on_fileListWidget_itemSelectionChanged()
{
    loadLogFiles();
}

/**
 * Adds an ignore pattern from the filter text
 */
void MainWindow::on_ignorePatternFilterLineEdit_returnPressed()
{
    // add ignore pattern
    addIgnorePattern(ui->ignorePatternFilterLineEdit->text());
}

/**
 * Adds a report pattern from the filter text
 */
void MainWindow::on_reportPatternFilterLineEdit_returnPressed()
{
    // add report pattern
    addReportPattern(ui->reportPatternFilterLineEdit->text());
}

void MainWindow::on_action_Settings_triggered()
{
    // open the settings dialog
    openSettingsDialog();
}

/**
 * @brief Opens the settings dialog
 */
void MainWindow::openSettingsDialog(int tab) {
    // open the settings dialog
    SettingsDialog *dialog = new SettingsDialog(tab, this);
    int dialogResult = dialog->exec();

    if (dialogResult == QDialog::Accepted) {
    }
    
    // read all relevant settings, that can be set in the settings dialog
    readSettingsFromSettingsDialog();
}

/**
 * @brief Reads all relevant settings, that can be set in the settings dialog
 */
void MainWindow::readSettingsFromSettingsDialog() {
//    QSettings settings;

    int logFileSourceCount = LogFileSource::countAll();
    qDebug() << __func__ << " - 'logFileSourceCount': " << logFileSourceCount;

    ui->logFileSourceFrame->setVisible(logFileSourceCount > 0);

    // remove all filter watchers
    setLocalDirectoryWatcherPath("");

    updateLogFileSourceComboBox();
}

void MainWindow::updateLogFileSourceComboBox() {
    ui->logFileSourceComboBox->clear();

    QList<LogFileSource> logFileSources = LogFileSource::fetchAll();
    int logFileSourcesCount = logFileSources.count();

    // populate the log file source combo box
    if (logFileSourcesCount > 0) {
        int currentId = LogFileSource::activeLogFileSourceId();
        int activeIndex = 0;
        int index = 0;

        Q_FOREACH(LogFileSource logFileSource, logFileSources) {
                // add an entry to the combo box
                ui->logFileSourceComboBox->addItem(logFileSource.getName(),
                                                   logFileSource.getId());
                if (currentId == logFileSource.getId()) {
                    activeIndex = index;
                }

                index++;
            }

        // set the active element
        ui->logFileSourceComboBox->setCurrentIndex(activeIndex);
    }
}

void MainWindow::on_logFileSourceComboBox_currentIndexChanged(int index)
{
    int logFileSourceId = ui->logFileSourceComboBox->itemData(index).toInt();
    LogFileSource logFileSource = LogFileSource::fetch(logFileSourceId);
    if (logFileSource.isFetched()) {
        changeLogFileSource(logFileSource);
    }

}

void MainWindow::changeLogFileSource(LogFileSource logFileSource)
{
    logFileSource.setAsActive();
    int type = logFileSource.getType();
    ui->logFileSourceStackedWidget->setCurrentIndex(type - 1);

    // remove all watcher paths
    setLocalDirectoryWatcherPath("");

    if (type == LogFileSource::LocalType) {
        QString localPath = logFileSource.getLocalPath();
        setLocalDirectoryWatcherPath(localPath);
        loadLocalLogFileSourceFiles(localPath);
    } else if (type == LogFileSource::EzPublishServerType) {
        on_logFileSourceRemoteReloadButton_clicked();
    }
}

void MainWindow::setLocalDirectoryWatcherPath(QString path)
{
    // clear all paths from the directory watcher
    _localDirectoryWatcher->removePaths(_localDirectoryWatcher->directories());
    _localDirectoryWatcher->removePaths(_localDirectoryWatcher->files());

    if (!path.isEmpty() && QDir(path).exists()) {
        _localDirectoryWatcher->addPath(path);
    }
}

void MainWindow::loadLocalLogFileSourceFiles(QString localPath)
{
    qDebug() << __func__ << " - 'localPath': " << localPath;

    QDir dir(localPath);

    // only show log files
    QStringList filters;
    filters << "*.log*";

    QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);

    ui->localFilesListWidget->clear();
    Q_FOREACH(QString fileName, files) {
            QListWidgetItem *item = new QListWidgetItem(fileName);
            QString filePath =
                    dir.absolutePath() + QDir::separator() + fileName;
            item->setData(Qt::UserRole, filePath);
            item->setToolTip(filePath);
            ui->localFilesListWidget->addItem(item);
        }
}

/**
 * Adds multiple local files to the file list
 */
void MainWindow::on_localFileUsePushButton_clicked()
{
    Q_FOREACH(QListWidgetItem *item,
              ui->localFilesListWidget->selectedItems()) {
            QString filePath = item->data(Qt::UserRole).toString();
            addPathToFileListWidget(filePath);
        }
}

/**
 * Adds a single local file to the file list
 */
void MainWindow::on_localFilesListWidget_itemDoubleClicked(
        QListWidgetItem *item)
{
    QString filePath = item->data(Qt::UserRole).toString();
    addPathToFileListWidget(filePath);
}

/**
 * Adds a path to the log file list widget if it doesn't already exist there
 */
void MainWindow::addPathToFileListWidget(QString path)
{
    if (ui->fileListWidget->findItems(path, Qt::MatchExactly).count() == 0) {
        ui->fileListWidget->addItem(path);
    } else {
        ui->statusBar->showMessage(tr("%1 was already in the list").arg(path));
    }
}

/**
 * Loads the log file list from the server and calls
 * fillEzPublishRemoteFilesListWidget with the results
 */
void MainWindow::on_logFileSourceRemoteReloadButton_clicked()
{
    ui->statusBar->showMessage(tr("Fetching file list from remote server"));
    EzPublishService *service = new EzPublishService(this);
    service->loadLogFileList(this);
}

/**
 * Shows the files from a string list in the eZPublishRemoteFilesListWidget
 */
void MainWindow::fillEzPublishRemoteFilesListWidget(QJsonArray fileList)
{
    ui->eZPublishRemoteFilesListWidget->clear();
    ui->eZPublishRemoteFilesTableWidget->clear();

    ui->eZPublishRemoteFilesTableWidget->setRowCount(fileList.count());

    QTableWidgetItem *nameHeader = new QTableWidgetItem(tr("File name"));
    ui->eZPublishRemoteFilesTableWidget->setHorizontalHeaderItem(0, nameHeader);

    QTableWidgetItem *sizeHeader = new QTableWidgetItem(tr("File size"));
    ui->eZPublishRemoteFilesTableWidget->setHorizontalHeaderItem(1, sizeHeader);

    QTableWidgetItem *downloadHeader = new QTableWidgetItem(tr("Download"));
    ui->eZPublishRemoteFilesTableWidget->setHorizontalHeaderItem(
            2, downloadHeader);

    ui->eZPublishRemoteFilesTableWidget->horizontalHeader()
            ->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->eZPublishRemoteFilesTableWidget->horizontalHeader()
            ->setSectionResizeMode(1, QHeaderView::Interactive);
//            setResizeMode( 0, QHeaderView::Stretch );

    int i = 0;
    Q_FOREACH(QJsonValue jsonValue, fileList) {
            QJsonObject obj = jsonValue.toObject();
            QString fileName = obj.value("file_name").toString();
            int fileSize = obj.value("file_size").toInt();

            QListWidgetItem *item = new QListWidgetItem(fileName);
            item->setToolTip(
                    tr("<strong>%1</strong><br />size: %2").arg(
                            fileName, Utils::Misc::friendlyUnit(fileSize)));

            ui->eZPublishRemoteFilesListWidget->addItem(item);

            QTableWidgetItem *nameItem = new QTableWidgetItem(fileName);
            ui->eZPublishRemoteFilesTableWidget->setItem(i, 0, nameItem);

            // we use our custom table widget item for our custom sorting
            // mechanism
            FileSizeTableWidgetItem *sizeItem = new FileSizeTableWidgetItem();
            sizeItem->setData(Qt::UserRole, fileSize);
            sizeItem->setText(Utils::Misc::friendlyUnit(fileSize));
            sizeItem->setFlags(sizeItem->flags() & ~Qt::ItemIsSelectable);
            ui->eZPublishRemoteFilesTableWidget->setItem(i, 1, sizeItem);

            QTableWidgetItem *downloadItem = new QTableWidgetItem();
            downloadItem->setData(Qt::UserRole, fileName);
            downloadItem->setCheckState(Qt::Unchecked);
            downloadItem->setFlags(
                    (downloadItem->flags() | Qt::ItemIsUserCheckable)
                    & ~Qt::ItemIsSelectable);
            ui->eZPublishRemoteFilesTableWidget->setItem(i, 2, downloadItem);

            i++;
        }

    ui->statusBar->clearMessage();
}

/**
 * Downloads the selected log files from the eZ Publish server
 */
void MainWindow::on_logFileSourceRemoteDownloadButton_clicked()
{
    ui->statusBar->showMessage(tr("Downloading log files from remote server"),
                               2000);
    EzPublishService *service = new EzPublishService(this);

//    Q_FOREACH(QListWidgetItem *item,
//               ui->eZPublishRemoteFilesListWidget->selectedItems() ) {
//            service->downloadLogFile(this, item->text());
//        }

    Q_FOREACH(QTableWidgetItem *item,
               ui->eZPublishRemoteFilesTableWidget->selectedItems() ) {
            service->downloadLogFile(this, item->text());
        }
}
