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
#include <QPixmap>
#include <QPainter>
#include <QFileInfo>

// Stylesheet for invalid input fields
static const char* INVALID_FIELD_STYLE = "QLineEdit { border: 2px solid #ff6b6b; background-color: #ffe0e0; }";

//=============================================================================
// Page 0: Introduction
//=============================================================================
SEMPQIntroPage::SEMPQIntroPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Welcome to SEMPQ Creation Wizard");
    setSubTitle("Create Self-Executing MPQ (SEMPQ) files.");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(15);

    QLabel *introLabel = new QLabel(
        "<p>This wizard will help you create a Self-Executing MPQ (SEMPQ) file.</p>"

        "<p><b>What are MPQs?</b><br>"
        "MPQs are archives containing game data such as  graphics, sounds and other resources. "
        "They were used extensively by Blizzard Entertainment, but also Sierra OnLine's "
        "Lords of Magic.</p>"

        "<p><b>What is a SEMPQ?</b><br>"
        "A SEMPQ is a standalone executable that contains an MPQ archive and optional plugins. "
        "When run, it automatically patches and launches a game with the embedded modifications. "
        "The user does not need to have MPQDraft installed to run a SEMPQ.</p>"

        "<p><b>Benefits of SEMPQ files:</b></p>"
        "<ul>"
        "<li>Easy distribution - share a single .exe file with others</li>"
        "<li>No installation required - recipients just run the file</li>"
        "<li>Automatic patching - the game is patched and launched in one step</li>"
        "<li>Self-contained - includes all necessary MPQ data and plugins</li>"
        "</ul>"

        "<p>Click <b>Next</b> to configure your SEMPQ file.</p>",
        this
    );
    introLabel->setWordWrap(true);
    introLabel->setTextFormat(Qt::RichText);

    layout->addWidget(introLabel);
    layout->addStretch();
}

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
    connect(mpqPathEdit, &QLineEdit::textChanged, this, &SEMPQSettingsPage::onMPQPathChanged);
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

void SEMPQSettingsPage::validateMPQPath()
{
    QString mpqPath = mpqPathEdit->text().trimmed();

    if (mpqPath.isEmpty()) {
        mpqPathEdit->setStyleSheet("");
        mpqPathEdit->setToolTip("");
        return;
    }

    QFileInfo fileInfo(mpqPath);
    if (!fileInfo.exists()) {
        mpqPathEdit->setStyleSheet(INVALID_FIELD_STYLE);
        mpqPathEdit->setToolTip("File does not exist");
    } else if (!fileInfo.isFile()) {
        mpqPathEdit->setStyleSheet(INVALID_FIELD_STYLE);
        mpqPathEdit->setToolTip("Path is not a file");
    } else {
        mpqPathEdit->setStyleSheet("");
        mpqPathEdit->setToolTip("");
    }
}

void SEMPQSettingsPage::onMPQPathChanged()
{
    validateMPQPath();
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

    // Set the wizard sidebar image with margin
    QPixmap originalPixmap(":/images/wizard.png");
    int margin = 10;
    QPixmap pixmapWithMargin(originalPixmap.width() + margin * 2,
                             originalPixmap.height() + margin * 2);
    pixmapWithMargin.fill(Qt::transparent);
    QPainter painter(&pixmapWithMargin);
    painter.drawPixmap(margin, margin, originalPixmap);
    painter.end();
    setPixmap(QWizard::WatermarkPixmap, pixmapWithMargin);

    // Create pages
    introPage = new SEMPQIntroPage(this);
    settingsPage = new SEMPQSettingsPage(this);
    pluginPage = new PluginPage(this);

    // Add pages
    addPage(introPage);
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
