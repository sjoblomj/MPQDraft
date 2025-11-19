/*
    PatchWizard - Implementation
*/

#include "patchwizard.h"
#include "pluginpage.h"
#include "common/patcher.h"
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
PatchIntroPage::PatchIntroPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Welcome to MPQDraft Patch Wizard");
    setSubTitle("Load custom MPQ archives with game data, or use plugins to add new features.");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(15);

    QLabel *introLabel = new QLabel(
        "<p>This wizard will help you patch a game executable with custom MPQ archives "
        "to change game assets and plugins to change game behavior.</p>"

        "<p><b>What are MPQs?</b><br>"
        "MPQs are archives containing game data such as  graphics, sounds and other resources. "
        "They were used extensively by Blizzard Entertainment, but also Sierra OnLine's "
        "Lords of Magic.</p>"

        "<p><b>What is MPQDraft?</b><br>"
        "MPQDraft allows you to modify games by loading custom MPQ files and plugins "
        "without permanently modifying the game installation.</p>"

        "<p><b>What you can do:</b></p>"
        "<ul>"
        "<li>Load custom graphics, sounds, and other game data from MPQ files provided by you</li>"
        "<li>Enable plugins that add new features or modify game behavior</li>"
        "<li>Launch the game with your modifications applied temporarily</li>"
        "</ul>"

        "<p><b>How it works:</b><br>"
        "MPQDraft intercepts the game's file access and redirects it to your custom MPQ files, "
        "allowing you to run modifications without altering the original game files.</p>"

        "<p>Click <b>Next</b> to begin selecting your target game and MPQ files.</p>",
        this
    );
    introLabel->setWordWrap(true);
    introLabel->setTextFormat(Qt::RichText);

    layout->addWidget(introLabel);
    layout->addStretch();
}

//=============================================================================
// Page 1: Target Selection
//=============================================================================
TargetSelectionPage::TargetSelectionPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Select Target Executable");
    setSubTitle("Choose the game executable to patch and any command-line parameters.");
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Target path
    QLabel *targetLabel = new QLabel("Target Executable:", this);
    layout->addWidget(targetLabel);
    
    QHBoxLayout *targetLayout = new QHBoxLayout();
    targetPathEdit = new QLineEdit(this);
    targetPathEdit->setPlaceholderText("Path to game executable (e.g., StarCraft.exe)");
    browseButton = new QPushButton("Browse...", this);
    connect(browseButton, &QPushButton::clicked, this, &TargetSelectionPage::onBrowseClicked);
    connect(targetPathEdit, &QLineEdit::textChanged, this, &TargetSelectionPage::onTargetPathChanged);
    targetLayout->addWidget(targetPathEdit);
    targetLayout->addWidget(browseButton);
    layout->addLayout(targetLayout);
    
    layout->addSpacing(20);
    
    // Parameters
    QLabel *paramsLabel = new QLabel("Command-line Parameters (optional):", this);
    layout->addWidget(paramsLabel);
    parametersEdit = new QLineEdit(this);
    parametersEdit->setPlaceholderText("e.g., -window -opengl");
    layout->addWidget(parametersEdit);
    
    layout->addSpacing(20);

    // Extended redirect option with info icon
    QHBoxLayout *extendedRedirLayout = new QHBoxLayout();

    extendedRedirCheck = new QCheckBox("Use extended file redirection", this);
    extendedRedirLayout->addWidget(extendedRedirCheck);

    // Info icon with detailed explanation
    QLabel *infoIcon = new QLabel(this);
    infoIcon->setText(" ? ");
    infoIcon->setStyleSheet(
        "QLabel { "
        "background-color: #0066cc; "
        "color: white; "
        "border-radius: 10px; "
        "font-weight: bold; "
        "font-size: 12px; "
        "padding: 2px; "
        "min-width: 16px; "
        "max-width: 16px; "
        "min-height: 16px; "
        "max-height: 16px; "
        "qproperty-alignment: AlignCenter; "
        "}");
    infoIcon->setToolTip(
        "<b>Extended File Redirection</b><br><br>"
        "Blizzard games use Storm.dll to access MPQ archives. Some Storm functions "
        "(like SFileOpenFileEx) can bypass the normal MPQ priority chain by accepting "
        "a specific archive handle.<br><br>"
        "When enabled, MPQDraft hooks these functions to force them to search through "
        "the entire MPQ priority chain (including your custom MPQs), even when the game "
        "tries to read from a specific archive.<br><br>"
        "<b>When to enable:</b> Most Blizzard games (StarCraft, Warcraft III, Diablo II) "
        "require this for mods to work correctly.<br><br>"
        "<b>When to disable:</b> Only disable if you're certain the target program doesn't "
        "use these Storm functions, or if you experience compatibility issues.");
    infoIcon->setCursor(Qt::WhatsThisCursor);
    extendedRedirLayout->addWidget(infoIcon);

    extendedRedirLayout->addStretch();
    layout->addLayout(extendedRedirLayout);

    layout->addStretch();
    
    // Register field for validation
    registerField("targetPath*", targetPathEdit);
}

QString TargetSelectionPage::getTargetPath() const
{
    return targetPathEdit->text();
}

QString TargetSelectionPage::getParameters() const
{
    return parametersEdit->text();
}

bool TargetSelectionPage::useExtendedRedir() const
{
    return extendedRedirCheck->isChecked();
}

void TargetSelectionPage::validateTargetPath()
{
    QString targetPath = targetPathEdit->text().trimmed();

    if (targetPath.isEmpty()) {
        targetPathEdit->setStyleSheet("");
        targetPathEdit->setToolTip("");
        return;
    }

    QFileInfo fileInfo(targetPath);
    if (!fileInfo.exists()) {
        targetPathEdit->setStyleSheet(INVALID_FIELD_STYLE);
        targetPathEdit->setToolTip("File does not exist");
    } else if (!fileInfo.isFile()) {
        targetPathEdit->setStyleSheet(INVALID_FIELD_STYLE);
        targetPathEdit->setToolTip("Path is not a file");
    } else {
        targetPathEdit->setStyleSheet("");
        targetPathEdit->setToolTip("");
    }
}

void TargetSelectionPage::onTargetPathChanged()
{
    validateTargetPath();
}

void TargetSelectionPage::onBrowseClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select Target Executable",
        QString(),
        "Executables (*.exe);;All Files (*.*)"
    );

    if (!fileName.isEmpty()) {
        targetPathEdit->setText(fileName);
    }
}

//=============================================================================
// Page 2: MPQ Selection
//=============================================================================
MPQSelectionPage::MPQSelectionPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Select MPQ Files");
    setSubTitle("Add MPQ files to load. Files are loaded in order (later files have higher priority).");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Status label at top
    statusLabel = new QLabel(this);
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    mainLayout->addWidget(statusLabel);

    // Horizontal layout for list and buttons
    QHBoxLayout *contentLayout = new QHBoxLayout();

    // MPQ list
    mpqListWidget = new QListWidget(this);
    connect(mpqListWidget, &QListWidget::itemChanged, this, &MPQSelectionPage::onItemChanged);
    contentLayout->addWidget(mpqListWidget);

    // Buttons
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    addButton = new QPushButton("Add...", this);
    removeButton = new QPushButton("Remove", this);
    moveUpButton = new QPushButton("Move Up", this);
    moveDownButton = new QPushButton("Move Down", this);

    connect(addButton, &QPushButton::clicked, this, &MPQSelectionPage::onAddClicked);
    connect(removeButton, &QPushButton::clicked, this, &MPQSelectionPage::onRemoveClicked);
    connect(moveUpButton, &QPushButton::clicked, this, &MPQSelectionPage::onMoveUpClicked);
    connect(moveDownButton, &QPushButton::clicked, this, &MPQSelectionPage::onMoveDownClicked);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(moveUpButton);
    buttonLayout->addWidget(moveDownButton);
    buttonLayout->addStretch();

    contentLayout->addLayout(buttonLayout);
    mainLayout->addLayout(contentLayout);
}

QStringList MPQSelectionPage::getSelectedMPQs() const
{
    QStringList mpqs;
    for (int i = 0; i < mpqListWidget->count(); ++i) {
        mpqs.append(mpqListWidget->item(i)->text());
    }
    return mpqs;
}

void MPQSelectionPage::validateMPQList()
{
    int mpqCount = mpqListWidget->count();

    // Check if too many MPQs
    if (mpqCount > MAX_PATCH_MPQS) {
        statusLabel->setText(QString("<font color='#d32f2f'><b>Warning:</b> Too many MPQ files (%1/%2). "
                                     "Please remove some files.</font>")
                            .arg(mpqCount).arg(MAX_PATCH_MPQS));
        statusLabel->show();
        return;
    }

    // Check for missing files
    QStringList missingFiles;
    for (int i = 0; i < mpqCount; ++i) {
        QString mpqPath = mpqListWidget->item(i)->text();
        QFileInfo fileInfo(mpqPath);

        if (!fileInfo.exists()) {
            missingFiles.append(QFileInfo(mpqPath).fileName());
            mpqListWidget->item(i)->setForeground(QColor("#d32f2f"));
            mpqListWidget->item(i)->setToolTip("File does not exist");
        } else {
            mpqListWidget->item(i)->setForeground(QColor());
            mpqListWidget->item(i)->setToolTip("");
        }
    }

    if (!missingFiles.isEmpty()) {
        statusLabel->setText(QString("<font color='#d32f2f'><b>Warning:</b> Some MPQ files do not exist: %1</font>")
                            .arg(missingFiles.join(", ")));
        statusLabel->show();
    } else {
        statusLabel->hide();
    }
}

void MPQSelectionPage::onItemChanged()
{
    validateMPQList();
}

void MPQSelectionPage::onAddClicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        "Select MPQ Files",
        QString(),
        "MPQ Archives (*.mpq);;All Files (*.*)"
    );

    for (const QString &fileName : fileNames) {
        mpqListWidget->addItem(fileName);
    }

    validateMPQList();
}

void MPQSelectionPage::onRemoveClicked()
{
    QList<QListWidgetItem*> selected = mpqListWidget->selectedItems();
    for (QListWidgetItem *item : selected) {
        delete item;
    }
    validateMPQList();
}

void MPQSelectionPage::onMoveUpClicked()
{
    int currentRow = mpqListWidget->currentRow();
    if (currentRow > 0) {
        QListWidgetItem *item = mpqListWidget->takeItem(currentRow);
        mpqListWidget->insertItem(currentRow - 1, item);
        mpqListWidget->setCurrentRow(currentRow - 1);
    }
}

void MPQSelectionPage::onMoveDownClicked()
{
    int currentRow = mpqListWidget->currentRow();
    if (currentRow >= 0 && currentRow < mpqListWidget->count() - 1) {
        QListWidgetItem *item = mpqListWidget->takeItem(currentRow);
        mpqListWidget->insertItem(currentRow + 1, item);
        mpqListWidget->setCurrentRow(currentRow + 1);
    }
}

//=============================================================================
// Main Wizard
//=============================================================================
PatchWizard::PatchWizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle("MPQDraft Patch Wizard");
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
    introPage = new PatchIntroPage(this);
    targetPage = new TargetSelectionPage(this);
    mpqPage = new MPQSelectionPage(this);
    pluginPage = new PluginPage(this);

    // Add pages
    addPage(introPage);
    addPage(targetPage);
    addPage(mpqPage);
    addPage(pluginPage);

    // Set minimum size
    setMinimumSize(600, 550);
}

void PatchWizard::accept()
{
    performPatch();
    QWizard::accept();
}

void PatchWizard::performPatch()
{
    // Get data from pages
    QString targetPath = targetPage->getTargetPath();
    QString parameters = targetPage->getParameters();
    bool extendedRedir = targetPage->useExtendedRedir();
    QStringList mpqs = mpqPage->getSelectedMPQs();
    QStringList plugins = pluginPage->getSelectedPlugins();
    
    // TODO: Call the actual patcher DLL
    // For now, just show a message
    QString message = QString("Patch Configuration:\n\n"
                             "Target: %1\n"
                             "Parameters: %2\n"
                             "Extended Redir: %3\n"
                             "MPQs: %4\n"
                             "Plugins: %5")
                        .arg(targetPath)
                        .arg(parameters.isEmpty() ? "(none)" : parameters)
                        .arg(extendedRedir ? "Yes" : "No")
                        .arg(QString::number(mpqs.count()))
                        .arg(QString::number(plugins.count()));
    
    QMessageBox::information(this, "Patch Ready", 
                            message + "\n\nPatching functionality will be implemented next.");
}
