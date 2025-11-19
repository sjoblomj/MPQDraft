/*
    SEMPQWizard - Implementation
*/

#include "sempqwizard.h"
#include "pluginpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>

//=============================================================================
// Page 1: SEMPQ Settings
//=============================================================================
SEMPQSettingsPage::SEMPQSettingsPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("SEMPQ Settings");
    setSubTitle("Configure the self-executing MPQ file to create.");
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // SEMPQ name
    QLabel *nameLabel = new QLabel("SEMPQ Name:", this);
    layout->addWidget(nameLabel);
    sempqNameEdit = new QLineEdit(this);
    sempqNameEdit->setPlaceholderText("e.g., MyMod");
    layout->addWidget(sempqNameEdit);
    
    layout->addSpacing(20);
    
    // Source MPQ
    QLabel *mpqLabel = new QLabel("Source MPQ File:", this);
    layout->addWidget(mpqLabel);
    QHBoxLayout *mpqLayout = new QHBoxLayout();
    mpqPathEdit = new QLineEdit(this);
    mpqPathEdit->setPlaceholderText("Select the MPQ file to package");
    browseMPQButton = new QPushButton("Browse...", this);
    connect(browseMPQButton, &QPushButton::clicked, this, &SEMPQSettingsPage::onBrowseMPQClicked);
    mpqLayout->addWidget(mpqPathEdit);
    mpqLayout->addWidget(browseMPQButton);
    layout->addLayout(mpqLayout);
    
    layout->addSpacing(20);
    
    // Icon
    QLabel *iconLabel = new QLabel("Icon (optional):", this);
    layout->addWidget(iconLabel);
    QHBoxLayout *iconLayout = new QHBoxLayout();
    iconPathEdit = new QLineEdit(this);
    iconPathEdit->setPlaceholderText("Select an icon file (.ico)");
    browseIconButton = new QPushButton("Browse...", this);
    connect(browseIconButton, &QPushButton::clicked, this, &SEMPQSettingsPage::onBrowseIconClicked);
    iconLayout->addWidget(iconPathEdit);
    iconLayout->addWidget(browseIconButton);
    layout->addLayout(iconLayout);
    
    layout->addStretch();
    
    // Register required fields
    registerField("sempqName*", sempqNameEdit);
    registerField("mpqPath*", mpqPathEdit);
}

QString SEMPQSettingsPage::getSEMPQName() const
{
    return sempqNameEdit->text();
}

QString SEMPQSettingsPage::getMPQPath() const
{
    return mpqPathEdit->text();
}

QString SEMPQSettingsPage::getIconPath() const
{
    return iconPathEdit->text();
}

void SEMPQSettingsPage::onBrowseMPQClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select MPQ File",
        QString(),
        "MPQ Archives (*.mpq);;All Files (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        mpqPathEdit->setText(fileName);
    }
}

void SEMPQSettingsPage::onBrowseIconClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select Icon File",
        QString(),
        "Icon Files (*.ico);;All Files (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        iconPathEdit->setText(fileName);
    }
}

//=============================================================================
// Main Wizard
//=============================================================================
SEMPQWizard::SEMPQWizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle("MPQDraft SEMPQ Wizard");
    setWizardStyle(QWizard::ModernStyle);
    setOption(QWizard::HaveHelpButton, false);
    
    // Create pages
    settingsPage = new SEMPQSettingsPage(this);
    pluginPage = new PluginPage(this);
    
    // Add pages
    addPage(settingsPage);
    addPage(pluginPage);
    
    // Set minimum size
    setMinimumSize(600, 500);
}

void SEMPQWizard::accept()
{
    createSEMPQ();
    QWizard::accept();
}

void SEMPQWizard::createSEMPQ()
{
    // Get data from pages
    QString sempqName = settingsPage->getSEMPQName();
    QString mpqPath = settingsPage->getMPQPath();
    QString iconPath = settingsPage->getIconPath();
    QStringList plugins = pluginPage->getSelectedPlugins();
    
    // TODO: Call the actual SEMPQ creation code
    // For now, just show a message
    QString message = QString("SEMPQ Configuration:\n\n"
                             "Name: %1\n"
                             "Source MPQ: %2\n"
                             "Icon: %3\n"
                             "Plugins: %4")
                        .arg(sempqName)
                        .arg(mpqPath)
                        .arg(iconPath.isEmpty() ? "(default)" : iconPath)
                        .arg(QString::number(plugins.count()));
    
    QMessageBox::information(this, "SEMPQ Ready", 
                            message + "\n\nSEMPQ creation functionality will be implemented next.");
}

