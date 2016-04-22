#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <entities/logfilesource.h>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    enum SettingsTabs {
        LogFileSourcesTab
    };

    enum LogFileSourceTypes {
        LocalType,
        EzPublishServerType
    };

    explicit SettingsDialog(int tab, QWidget *parent = 0);
    ~SettingsDialog();

    void setConnectionTestMessage(QString text, bool isError = false);

private slots:
    void on_logFileSourceTypeComboBox_currentIndexChanged(int index);

    void on_logFileSourceAddButton_clicked();

    void on_logFileSourceRemoveButton_clicked();

    void on_logFileSourceNameLineEdit_editingFinished();

    void on_logFileSourceEzPublishServerUrlLineEdit_editingFinished();

    void on_logFileSourceEzPublishServerUsernameLineEdit_editingFinished();

    void on_logFileSourceEzPublishServerPasswordLineEdit_editingFinished();

    void on_logFileSourceLocalPathButton_clicked();

    void on_logFileSourceActiveCheckBox_stateChanged(int arg1);

    void on_logFileSourceListWidget_currentItemChanged(
            QListWidgetItem *current, QListWidgetItem *previous);

    void on_logFileSourceAddDownloadedFilePrefixCheckBox_toggled(bool checked);

    void on_connectionTestButton_clicked();

private:
    Ui::SettingsDialog *ui;
    LogFileSource _selectedLogFileSource;

    void setupLogFileSourceTab();

    void updateLogFileSourceTab();
};

#endif // SETTINGSDIALOG_H
