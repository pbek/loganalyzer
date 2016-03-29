#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

SettingsDialog::SettingsDialog(int tab, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    Q_UNUSED(tab);

    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(tab);
    setupLogFileSourceTab();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}


/**
 * Does the log file source tab setup
 */
void SettingsDialog::setupLogFileSourceTab() {
    ui->logFileSourceTypeComboBox
            ->setCurrentIndex(SettingsDialog::LocalType);
    ui->logFileSourceTypeConfigStackedWidget
            ->setCurrentIndex(SettingsDialog::LocalType);

    QList<LogFileSource> logFileSources = LogFileSource::fetchAll();
    int logFileSourcesCount = logFileSources.count();

    // populate the note folder list
    if (logFileSourcesCount > 0) {
        Q_FOREACH(LogFileSource logFileSource, logFileSources) {
                QListWidgetItem *item =
                        new QListWidgetItem(logFileSource.getName());
                item->setData(Qt::UserRole,
                              logFileSource.getId());
                ui->logFileSourceListWidget->addItem(item);
            }

        // set the current row
        ui->logFileSourceListWidget->setCurrentRow(0);
    }

    // update the visibility of frames and buttons
    updateLogFileSourceTab();
}

/**
 * Update the visibility of frames and buttons of the log file source tab
 */
void SettingsDialog::updateLogFileSourceTab() {
    int logFileSourcesCount = LogFileSource::countAll();

    // enable the edit frame if there are items
    ui->logFileSourceEditFrame->setEnabled(logFileSourcesCount > 0);

    // disable the remove button if there is no item
    ui->logFileSourceRemoveButton->setEnabled(logFileSourcesCount > 0);
}

void SettingsDialog::on_logFileSourceTypeComboBox_currentIndexChanged(int index)
{
    ui->logFileSourceTypeConfigStackedWidget->setCurrentIndex(index);
    _selectedLogFileSource.setType(index + 1);
    _selectedLogFileSource.store();
}

void SettingsDialog::on_logFileSourceAddButton_clicked()
{
    _selectedLogFileSource = LogFileSource();
    _selectedLogFileSource.setName(tr("new source"));
    _selectedLogFileSource.setType(LogFileSource::LocalType);
    _selectedLogFileSource.setPriority(ui->logFileSourceListWidget->count());
    _selectedLogFileSource.store();

    if (_selectedLogFileSource.isFetched()) {
        QListWidgetItem *item =
                new QListWidgetItem(_selectedLogFileSource.getName());
        item->setData(Qt::UserRole, _selectedLogFileSource.getId());
        ui->logFileSourceListWidget->addItem(item);

        // set the current row
        ui->logFileSourceListWidget->setCurrentRow(
                ui->logFileSourceListWidget->count() - 1);

        // update the visibility of frames and buttons
        updateLogFileSourceTab();
    }

}

void SettingsDialog::on_logFileSourceRemoveButton_clicked()
{
    if (QMessageBox::information(
            this,
            tr("Remove log file source"),
            tr("Remove the current log file source <strong>%1</strong>?")
                    .arg(_selectedLogFileSource.getName()),
            tr("&Remove"), tr("&Cancel"), QString::null,
            0, 1) == 0) {
        bool wasCurrent = _selectedLogFileSource.isCurrent();

        // remove the log file source from the database
        _selectedLogFileSource.remove();

        const QSignalBlocker blocker(ui->logFileSourceListWidget);
        Q_UNUSED(blocker);

        // remove the list item
        ui->logFileSourceListWidget->takeItem(
                ui->logFileSourceListWidget->currentRow());

        // update the visibility of frames and buttons
        updateLogFileSourceTab();

        // if the removed log file source was the current source we set the
        // first log file source as new current one
        if (wasCurrent) {
            QList<LogFileSource> logFileSources = LogFileSource::fetchAll();
            if (logFileSources.count() > 0) {
                logFileSources[0].setAsCurrent();
            }
        }
    }
}

void SettingsDialog::on_logFileSourceNameLineEdit_editingFinished()
{
    QString text = ui->logFileSourceNameLineEdit->text();
    _selectedLogFileSource.setName(text);
    _selectedLogFileSource.store();

    ui->logFileSourceListWidget->currentItem()->setText(text);
}

void SettingsDialog::on_logFileSourceEzPublishServerUrlLineEdit_editingFinished()
{
    QString text = ui->logFileSourceEzPublishServerUrlLineEdit->text();
    _selectedLogFileSource.setEzpServerUrl(text);
    _selectedLogFileSource.store();
}

void SettingsDialog::on_logFileSourceEzPublishServerUsernameLineEdit_editingFinished()
{
    QString text = ui->logFileSourceEzPublishServerUsernameLineEdit->text();
    _selectedLogFileSource.setEzpUsername(text);
    _selectedLogFileSource.store();
}

void SettingsDialog::on_logFileSourceEzPublishServerPasswordLineEdit_editingFinished()
{
    QString text = ui->logFileSourceEzPublishServerPasswordLineEdit->text();
    _selectedLogFileSource.setEzpPassword(text);
    _selectedLogFileSource.store();
}

void SettingsDialog::on_logFileSourceLocalPathButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(
            this,
            tr("Please select the folder where your log files are stored"),
            _selectedLogFileSource.getLocalPath(),
            QFileDialog::ShowDirsOnly);

    QDir d = QDir(dir);

    if (d.exists() && (dir != "")) {
        ui->logFileSourceLocalPathLineEdit->setText(dir);
        _selectedLogFileSource.setLocalPath(dir);
        _selectedLogFileSource.store();
    }
}

void SettingsDialog::on_logFileSourceActiveCheckBox_stateChanged(int arg1)
{
    Q_UNUSED(arg1);

    if (!ui->logFileSourceActiveCheckBox->isChecked()) {
        const QSignalBlocker blocker(ui->logFileSourceActiveCheckBox);
        Q_UNUSED(blocker);
        ui->logFileSourceActiveCheckBox->setChecked(true);
    } else {
        _selectedLogFileSource.setAsCurrent();
    }
}

void SettingsDialog::on_logFileSourceListWidget_currentItemChanged(
        QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);

    int logFileSourceId = current->data(Qt::UserRole).toInt();
    _selectedLogFileSource = LogFileSource::fetch(logFileSourceId);
    if (_selectedLogFileSource.isFetched()) {
        ui->logFileSourceNameLineEdit->setText(
                _selectedLogFileSource.getName());
        ui->logFileSourceLocalPathLineEdit->setText(
                _selectedLogFileSource.getLocalPath());
        ui->logFileSourceEzPublishServerUrlLineEdit->setText(
                _selectedLogFileSource.getEzpServerUrl());
        ui->logFileSourceEzPublishServerUsernameLineEdit->setText(
                _selectedLogFileSource.getEzpUsername());
        ui->logFileSourceEzPublishServerPasswordLineEdit->setText(
                _selectedLogFileSource.getEzpPassword());
        ui->logFileSourceTypeComboBox->setCurrentIndex(
                _selectedLogFileSource.getType() - 1);

        const QSignalBlocker blocker(ui->logFileSourceActiveCheckBox);
        Q_UNUSED(blocker);
        ui->logFileSourceActiveCheckBox->setChecked(
                _selectedLogFileSource.isCurrent());
    }
}
