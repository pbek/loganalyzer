#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

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

private slots:
    void on_logFileSourceTypeComboBox_currentIndexChanged(int index);

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
