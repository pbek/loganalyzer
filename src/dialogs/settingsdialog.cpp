#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(int tab, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    Q_UNUSED(tab);

    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(tab);
    ui->logFileSourceTypeComboBox
            ->setCurrentIndex(LogFileSourceTypes::LocalType);
    ui->logFileSourceTypeConfigStackedWidget
            ->setCurrentIndex(LogFileSourceTypes::LocalType);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_logFileSourceTypeComboBox_currentIndexChanged(int index)
{
    ui->logFileSourceTypeConfigStackedWidget->setCurrentIndex(index);
}
