/*
    SEMPQWizard - Implementation
*/

#include "sempqwizard.h"
#include "pluginpage.h"
#include "common/gamedata.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QFileInfo>
#include <QIcon>
#include <QScrollArea>
#include <QFrame>

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

    // Source MPQ with icon to the left of both label and input
    QHBoxLayout *mpqSectionLayout = new QHBoxLayout();

    // MPQ icon on the left
    mpqIconLabel = new QLabel(this);
    QPixmap mpqPixmap(":/icons/MPQ.ico");
    mpqIconLabel->setPixmap(mpqPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    mpqIconLabel->setFixedSize(64, 64);
    mpqIconLabel->setAlignment(Qt::AlignTop);
    mpqSectionLayout->addWidget(mpqIconLabel);

    // Vertical layout for label and input
    QVBoxLayout *mpqVerticalLayout = new QVBoxLayout();
    QLabel *mpqLabel = new QLabel("Source MPQ File:", this);
    mpqVerticalLayout->addWidget(mpqLabel);

    QHBoxLayout *mpqInputLayout = new QHBoxLayout();
    mpqPathEdit = new QLineEdit(this);
    mpqPathEdit->setPlaceholderText("Select the MPQ file to package");
    browseMPQButton = new QPushButton("Browse...", this);
    connect(browseMPQButton, &QPushButton::clicked, this, &SEMPQSettingsPage::onBrowseMPQClicked);
    connect(mpqPathEdit, &QLineEdit::textChanged, this, &SEMPQSettingsPage::onMPQPathChanged);
    mpqInputLayout->addWidget(mpqPathEdit);
    mpqInputLayout->addWidget(browseMPQButton);
    mpqVerticalLayout->addLayout(mpqInputLayout);

    mpqSectionLayout->addLayout(mpqVerticalLayout);
    layout->addLayout(mpqSectionLayout);

    layout->addSpacing(20);

    // Icon with preview to the left of both label and input
    QHBoxLayout *iconSectionLayout = new QHBoxLayout();

    // Icon preview on the left
    iconPreviewLabel = new QLabel(this);
    iconPreviewLabel->setFixedSize(64, 64);
    iconPreviewLabel->setAlignment(Qt::AlignTop);
    iconSectionLayout->addWidget(iconPreviewLabel);

    // Vertical layout for label and input
    QVBoxLayout *iconVerticalLayout = new QVBoxLayout();
    QLabel *iconLabel = new QLabel("Icon of SEMPQ (optional):", this);
    iconVerticalLayout->addWidget(iconLabel);

    QHBoxLayout *iconInputLayout = new QHBoxLayout();
    iconPathEdit = new QLineEdit(this);
    iconPathEdit->setPlaceholderText("Select an icon file (.ico)");
    browseIconButton = new QPushButton("Browse...", this);
    connect(browseIconButton, &QPushButton::clicked, this, &SEMPQSettingsPage::onBrowseIconClicked);
    connect(iconPathEdit, &QLineEdit::textChanged, this, &SEMPQSettingsPage::onIconPathChanged);
    iconInputLayout->addWidget(iconPathEdit);
    iconInputLayout->addWidget(browseIconButton);
    iconVerticalLayout->addLayout(iconInputLayout);

    iconSectionLayout->addLayout(iconVerticalLayout);
    layout->addLayout(iconSectionLayout);

    layout->addStretch();

    // Register required fields
    registerField("sempqName*", sempqNameEdit);
    registerField("mpqPath*", mpqPathEdit);

    SEMPQSettingsPage::onIconPathChanged();
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

void SEMPQSettingsPage::onIconPathChanged()
{
    updateIconPreview();
}

void SEMPQSettingsPage::updateIconPreview()
{
    QString iconPath = iconPathEdit->text().trimmed();

    bool loadedIcon = false;
    if (!iconPath.isEmpty()) {
        // Try to load the custom icon
        QPixmap customPixmap(iconPath);
        if (!customPixmap.isNull()) {
            iconPreviewLabel->setPixmap(customPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            loadedIcon = true;
        }
    }
    if (!loadedIcon) {
        // No custom icon or failed to load it - show default StarDraft.ico
        QPixmap defaultPixmap(":/icons/StarDraft.ico");
        iconPreviewLabel->setPixmap(defaultPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

//=============================================================================
// Page 2: Select Target Program
//=============================================================================
SEMPQTargetPage::SEMPQTargetPage(QWidget *parent)
    : QWizardPage(parent), selectedComponent(nullptr), warnOnExtendedRedirChange(true)
{
    setTitle("Select Target Program");
    setSubTitle("Choose the program that the SEMPQ will launch.");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    // Create tab widget (don't connect signal yet - will do after all widgets are created)
    tabWidget = new QTabWidget(this);
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //=========================================================================
    // Tab 1: Supported Games (Registry-Based)
    //=========================================================================
    QWidget *supportedGamesTab = new QWidget();
    QVBoxLayout *gamesLayout = new QVBoxLayout(supportedGamesTab);

    QLabel *gamesInfoLabel = new QLabel(
        "Select a supported game from the list below. The SEMPQ will automatically "
        "locate the game on the user's computer using the Windows registry, making "
        "it portable across different installations.",
        supportedGamesTab);
    gamesInfoLabel->setWordWrap(true);
    gamesLayout->addWidget(gamesInfoLabel);

    gamesLayout->addSpacing(10);

    // Game list
    gameList = new QListWidget(supportedGamesTab);
    gameList->setIconSize(QSize(32, 32));
    connect(gameList, &QListWidget::currentItemChanged, this, &SEMPQTargetPage::onGameSelectionChanged);

    // Populate game list with all components from all games
    const QVector<SupportedGame>& games = getSupportedGames();
    for (const SupportedGame& game : games) {
        for (const GameComponent& component : game.components) {
            QListWidgetItem *item = new QListWidgetItem(gameList);

            // Display format: "Game Name - Component Name"
            QString displayText;
            if (game.components.size() == 1) {
                // Single component - just show game name
                displayText = game.gameName;
            } else {
                // Multiple components - show "Game - Component"
                displayText = QString("%1 - %2").arg(game.gameName, component.componentName);
            }
            item->setText(displayText);

            // Set icon
            QIcon icon(component.iconPath);
            item->setIcon(icon);

            // Store pointer to component in item data
            item->setData(Qt::UserRole, QVariant::fromValue((void*)&component));
        }
    }

    gamesLayout->addWidget(gameList);

    //=========================================================================
    // Tab 2: Custom Registry
    //=========================================================================
    QWidget *customRegistryTab = new QWidget();
    QVBoxLayout *customRegTabLayout = new QVBoxLayout(customRegistryTab);

    // Create a scroll area for the content
    QScrollArea *customRegScrollArea = new QScrollArea(customRegistryTab);
    customRegScrollArea->setWidgetResizable(true);
    customRegScrollArea->setFrameShape(QFrame::NoFrame);

    // Set background to match tab background (white)
    QPalette scrollPalette = customRegScrollArea->palette();
    scrollPalette.setColor(QPalette::Window, Qt::white);
    customRegScrollArea->setPalette(scrollPalette);
    customRegScrollArea->setAutoFillBackground(true);

    // Create the content widget that will go inside the scroll area
    QWidget *customRegContentWidget = new QWidget();
    QVBoxLayout *customRegLayout = new QVBoxLayout(customRegContentWidget);

    QLabel *customRegInfoLabel = new QLabel(
        "Specify a custom registry key for games not in the supported list. "
        "This is more portable than a hardcoded path, as it will work on any "
        "computer where the game is properly installed.",
        customRegContentWidget);
    customRegInfoLabel->setWordWrap(true);
    customRegLayout->addWidget(customRegInfoLabel);

    customRegLayout->addSpacing(10);

    // Registry Key
    QLabel *regKeyLabel = new QLabel("Registry Key:", customRegContentWidget);
    customRegLayout->addWidget(regKeyLabel);

    customRegKeyEdit = new QLineEdit(customRegContentWidget);
    customRegKeyEdit->setPlaceholderText("e.g., SOFTWARE\\Blizzard Entertainment\\Starcraft II");
    connect(customRegKeyEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::onCustomRegistryChanged);
    customRegLayout->addWidget(customRegKeyEdit);

    customRegLayout->addSpacing(5);

    // Registry Value
    QLabel *regValueLabel = new QLabel("Registry Value Name:", customRegContentWidget);
    customRegLayout->addWidget(regValueLabel);

    customRegValueEdit = new QLineEdit(customRegContentWidget);
    customRegValueEdit->setPlaceholderText("e.g., InstallPath");
    connect(customRegValueEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::onCustomRegistryChanged);
    customRegLayout->addWidget(customRegValueEdit);

    customRegLayout->addSpacing(5);

    // Executable Filename
    QLabel *exeFileLabel = new QLabel("Executable Filename:", customRegContentWidget);
    customRegLayout->addWidget(exeFileLabel);

    customRegExeEdit = new QLineEdit(customRegContentWidget);
    customRegExeEdit->setPlaceholderText("e.g., StarCraft II.exe");
    connect(customRegExeEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::onCustomRegistryChanged);
    customRegLayout->addWidget(customRegExeEdit);

    customRegLayout->addSpacing(5);

    // Target File Name (optional)
    QLabel *targetFileLabel = new QLabel("Target File Name (optional):", customRegContentWidget);
    customRegLayout->addWidget(targetFileLabel);

    customRegTargetFileEdit = new QLineEdit(customRegContentWidget);
    customRegTargetFileEdit->setPlaceholderText("Leave empty if same as executable (most common)");
    customRegTargetFileEdit->setToolTip(
        "The file to patch. Usually the same as the executable.\n"
        "Only specify if different (e.g., Diablo II launches 'Diablo II.exe' but patches 'Game.exe').");
    customRegLayout->addWidget(customRegTargetFileEdit);

    customRegLayout->addSpacing(5);

    // Shunt Count
    QLabel *shuntCountLabel = new QLabel("Shunt Count:", customRegContentWidget);
    customRegLayout->addWidget(shuntCountLabel);

    customRegShuntCountSpinBox = new QSpinBox(customRegContentWidget);
    customRegShuntCountSpinBox->setMinimum(0);
    customRegShuntCountSpinBox->setMaximum(INT_MAX);  // Unlimited (max int value)
    customRegShuntCountSpinBox->setValue(0);
    customRegShuntCountSpinBox->setToolTip(
        "Number of times the game restarts itself before MPQDraft activates.\n"
        "0 = Activate immediately (most games)\n"
        "1 = Wait for one restart (games with copy protection like Diablo)");
    customRegLayout->addWidget(customRegShuntCountSpinBox);

    customRegLayout->addSpacing(10);

    // Checkbox for "Value is full path"
    customRegIsFullPathCheckbox = new QCheckBox(
        "Registry value contains full path to executable (not just directory)",
        customRegContentWidget);
    customRegIsFullPathCheckbox->setToolTip(
        "Check this if the registry value is the full path to the .exe file.\n"
        "Leave unchecked if the registry value is just the installation directory.");
    customRegLayout->addWidget(customRegIsFullPathCheckbox);

    customRegLayout->addSpacing(10);

    // Advanced flags section
    QLabel *flagsLabel = new QLabel("<b>Advanced Flags:</b>", customRegContentWidget);
    customRegLayout->addWidget(flagsLabel);

    customRegNoSpawningCheckbox = new QCheckBox(
        "Do not inject into child processes (MPQD_NO_SPAWNING)",
        customRegContentWidget);
    customRegNoSpawningCheckbox->setToolTip(
        "Prevents MPQDraft from injecting into child processes created by the game.\n"
        "Use this if the game launches helper processes that shouldn't be patched.");
    customRegLayout->addWidget(customRegNoSpawningCheckbox);

    customRegLayout->addStretch();

    // Set the content widget in the scroll area and add scroll area to tab
    customRegScrollArea->setWidget(customRegContentWidget);
    customRegTabLayout->addWidget(customRegScrollArea);

    //=========================================================================
    // Tab 3: Custom Target (Hardcoded Path)
    //=========================================================================
    QWidget *customTargetTab = new QWidget();
    QVBoxLayout *customLayout = new QVBoxLayout(customTargetTab);

    QLabel *customInfoLabel = new QLabel(
        "Specify a custom program path. This can be used for programs not in the "
        "supported games list.",
        customTargetTab);
    customInfoLabel->setWordWrap(true);
    customLayout->addWidget(customInfoLabel);

    customLayout->addSpacing(10);

    // Warning label
    warningLabel = new QLabel(
        "<b>⚠ Warning:</b> When using a custom path, the exact path you specify must "
        "exist on all computers where the SEMPQ will be run. For better portability, "
        "use the Supported Games tab instead.",
        customTargetTab);
    warningLabel->setWordWrap(true);
    warningLabel->setStyleSheet(
        "QLabel { "
        "background-color: #fff3cd; "
        "border: 1px solid #ffc107; "
        "border-radius: 4px; "
        "padding: 10px; "
        "color: #856404; "
        "}");
    customLayout->addWidget(warningLabel);

    customLayout->addSpacing(10);

    // Target path
    QLabel *targetLabel = new QLabel("Target Program Path:", customTargetTab);
    customLayout->addWidget(targetLabel);

    QHBoxLayout *targetLayout = new QHBoxLayout();
    customPathEdit = new QLineEdit(customTargetTab);
    customPathEdit->setPlaceholderText("Path to the executable (can be relative)");
    connect(customPathEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::onCustomPathChanged);

    browseButton = new QPushButton("Browse...", customTargetTab);
    connect(browseButton, &QPushButton::clicked, this, &SEMPQTargetPage::onBrowseClicked);

    targetLayout->addWidget(customPathEdit);
    targetLayout->addWidget(browseButton);
    customLayout->addLayout(targetLayout);

    customLayout->addStretch();

    //=========================================================================
    // Add tabs to tab widget
    //=========================================================================
    tabWidget->addTab(supportedGamesTab, "Supported &Games");
    tabWidget->addTab(customRegistryTab, "Custom &Registry");
    tabWidget->addTab(customTargetTab, "Custom &Target");

    mainLayout->addWidget(tabWidget);

    mainLayout->addSpacing(10);

    //=========================================================================
    // Common controls (below tabs)
    //=========================================================================
    QLabel *paramsLabel = new QLabel("Command-Line Parameters (optional):", this);
    mainLayout->addWidget(paramsLabel);

    parametersEdit = new QLineEdit(this);
    parametersEdit->setPlaceholderText("e.g., -window -opengl");
    mainLayout->addWidget(parametersEdit);

    mainLayout->addSpacing(10);

    // Extended redir checkbox (common to both modes)
    QHBoxLayout *extendedRedirLayout = new QHBoxLayout();

    extendedRedirCheckbox = new QCheckBox("Redirect SFileOpenFileEx calls", this);
    extendedRedirCheckbox->setChecked(true);  // Default to true
    extendedRedirLayout->addWidget(extendedRedirCheckbox);

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
    mainLayout->addLayout(extendedRedirLayout);

    // Now that all widgets are created, connect the signals
    connect(tabWidget, &QTabWidget::currentChanged, this, &SEMPQTargetPage::onTabChanged);
    connect(extendedRedirCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onExtendedRedirChanged);
}

bool SEMPQTargetPage::isRegistryBased() const
{
    int index = tabWidget->currentIndex();
    return (index == 0 || index == 1);  // Tab 0: Supported Games, Tab 1: Custom Registry
}

const GameComponent* SEMPQTargetPage::getSelectedComponent() const
{
    if (!isRegistryBased()) {
        return nullptr;
    }
    return selectedComponent;
}

QString SEMPQTargetPage::getCustomTargetPath() const
{
    if (isRegistryBased()) {
        return QString();
    }
    return customPathEdit->text();
}

QString SEMPQTargetPage::getParameters() const
{
    return parametersEdit->text();
}

QString SEMPQTargetPage::getCustomRegistryKey() const
{
    return customRegKeyEdit->text().trimmed();
}

QString SEMPQTargetPage::getCustomRegistryValue() const
{
    return customRegValueEdit->text().trimmed();
}

QString SEMPQTargetPage::getCustomRegistryExe() const
{
    return customRegExeEdit->text().trimmed();
}

QString SEMPQTargetPage::getCustomRegistryTargetFile() const
{
    return customRegTargetFileEdit->text().trimmed();
}

int SEMPQTargetPage::getCustomRegistryShuntCount() const
{
    return customRegShuntCountSpinBox->value();
}

bool SEMPQTargetPage::getCustomRegistryIsFullPath() const
{
    return customRegIsFullPathCheckbox->isChecked();
}

uint32_t SEMPQTargetPage::getCustomRegistryFlags() const
{
    uint32_t flags = 0;

    // Extended redirection is handled by the common checkbox
    if (extendedRedirCheckbox->isChecked()) {
        flags |= MPQD_EXTENDED_REDIR;
    }

    // Custom registry specific flag
    if (customRegNoSpawningCheckbox->isChecked()) {
        flags |= MPQD_NO_SPAWNING;
    }

    return flags;
}

bool SEMPQTargetPage::getExtendedRedir() const
{
    int index = tabWidget->currentIndex();

    if (index == 0 && selectedComponent) {
        // Tab 0: Supported Games - use the component's default
        return selectedComponent->extendedRedir;
    } else {
        // Tab 1: Custom Registry or Tab 2: Custom Target - use the checkbox
        return extendedRedirCheckbox->isChecked();
    }
}

void SEMPQTargetPage::onGameSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);

    if (!current) {
        selectedComponent = nullptr;
        return;
    }

    // Retrieve the component pointer from item data
    void* ptr = current->data(Qt::UserRole).value<void*>();
    selectedComponent = static_cast<const GameComponent*>(ptr);

    // Update extended redir checkbox to show the default for this component
    updateExtendedRedirCheckbox();

    // Emit completeChanged to update wizard buttons
    emit completeChanged();
}

void SEMPQTargetPage::onBrowseClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select Target Program",
        QString(),
        "Executable Files (*.exe);;All Files (*)"
    );

    if (!fileName.isEmpty()) {
        customPathEdit->setText(fileName);
    }
}

void SEMPQTargetPage::onCustomPathChanged()
{
    validateCustomPath();
    emit completeChanged();
}

void SEMPQTargetPage::onCustomRegistryChanged()
{
    emit completeChanged();
}

void SEMPQTargetPage::onTabChanged(int index)
{
    updateExtendedRedirCheckbox();
    emit completeChanged();
}

void SEMPQTargetPage::validateCustomPath()
{
    QString path = customPathEdit->text();

    if (path.isEmpty()) {
        customPathEdit->setStyleSheet("");
        customPathEdit->setToolTip("");
        return;
    }

    // For SEMPQ wizard, we allow relative paths and non-existent files
    // (since the target might not exist on the machine creating the SEMPQ)
    // So we only do basic validation

    QFileInfo fileInfo(path);

    // If it's an absolute path and exists, check if it's a file
    if (fileInfo.isAbsolute() && fileInfo.exists() && !fileInfo.isFile()) {
        customPathEdit->setStyleSheet(INVALID_FIELD_STYLE);
        customPathEdit->setToolTip("Path is not a file");
    } else {
        customPathEdit->setStyleSheet("");
        customPathEdit->setToolTip("");
    }
}

void SEMPQTargetPage::updateExtendedRedirCheckbox()
{
    if (!extendedRedirCheckbox) {
        return;  // Not initialized yet
    }

    // Temporarily disconnect to avoid triggering warning when we update programmatically
    disconnect(extendedRedirCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onExtendedRedirChanged);

    if (isRegistryBased() && selectedComponent) {
        // Set to the default for the selected component
        extendedRedirCheckbox->setChecked(selectedComponent->extendedRedir);
        // Reset warning flag when changing games
        warnOnExtendedRedirChange = true;
    } else {
        // Custom target - default to true (most games need it)
        extendedRedirCheckbox->setChecked(true);
        warnOnExtendedRedirChange = true;
    }

    // Reconnect the signal
    connect(extendedRedirCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onExtendedRedirChanged);
}

void SEMPQTargetPage::onExtendedRedirChanged(int state)
{
    Q_UNUSED(state);

    // Only warn if we have a selection and haven't warned yet
    if (!warnOnExtendedRedirChange) {
        return;
    }

    // For registry-based, only warn if there's a selected component
    // For custom target, always warn on first change
    if (isRegistryBased() && !selectedComponent) {
        return;
    }

    // Show warning dialog
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("Extended File Redirection");
    msgBox.setText("It is highly recommended that you do not change 'Redirect SFileOpenFileEx calls' "
                   "unless you are completely sure what you are doing. Do you wish to continue?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    int result = msgBox.exec();

    if (result == QMessageBox::Yes) {
        // User confirmed - don't warn again
        warnOnExtendedRedirChange = false;
    } else {
        // User cancelled - revert the change
        disconnect(extendedRedirCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onExtendedRedirChanged);
        extendedRedirCheckbox->setChecked(!extendedRedirCheckbox->isChecked());
        connect(extendedRedirCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onExtendedRedirChanged);
    }
}

bool SEMPQTargetPage::isComplete() const
{
    int index = tabWidget->currentIndex();

    if (index == 0) {
        // Tab 0: Supported Games - Must have a game selected
        return selectedComponent != nullptr;
    } else if (index == 1) {
        // Tab 1: Custom Registry - Must have all fields filled
        return !customRegKeyEdit->text().trimmed().isEmpty() &&
               !customRegValueEdit->text().trimmed().isEmpty() &&
               !customRegExeEdit->text().trimmed().isEmpty();
    } else {
        // Tab 2: Custom Target - Must have a custom path entered
        return !customPathEdit->text().isEmpty();
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

    // Set the wizard sidebar image with margin and frame
    QPixmap originalPixmap(":/images/wizard.png");
    int innerMargin = 10;  // Space between frame and image
    int outerMargin = 25;  // Space between canvas edge and frame
    int frameWidth = 1;

    // Calculate total canvas size
    QPixmap pixmapWithMargin(originalPixmap.width() + innerMargin * 2 + frameWidth * 2 + outerMargin * 2,
                             originalPixmap.height() + innerMargin * 2 + frameWidth * 2 + outerMargin * 2);
    pixmapWithMargin.fill(Qt::transparent);
    QPainter painter(&pixmapWithMargin);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Calculate positions
    int frameX = outerMargin;
    int frameY = outerMargin;
    int frameRectWidth = originalPixmap.width() + innerMargin * 2 + frameWidth * 2;
    int frameRectHeight = originalPixmap.height() + innerMargin * 2 + frameWidth * 2;
    int imageX = outerMargin + frameWidth + innerMargin;
    int imageY = outerMargin + frameWidth + innerMargin;

    // Draw the background inside the frame (light gray)
    painter.fillRect(frameX + frameWidth, frameY + frameWidth,
                     frameRectWidth - frameWidth * 2, frameRectHeight - frameWidth * 2,
                     QColor(200, 200, 200));

    // Draw the frame border
    painter.setPen(QPen(QColor(100, 100, 100), frameWidth));  // Dark gray
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(frameX, frameY, frameRectWidth - 1, frameRectHeight - 1);

    // Draw the image inside the frame
    painter.drawPixmap(imageX, imageY, originalPixmap);

    painter.end();
    setPixmap(QWizard::WatermarkPixmap, pixmapWithMargin);

    // Create pages
    introPage = new SEMPQIntroPage(this);
    settingsPage = new SEMPQSettingsPage(this);
    targetPage = new SEMPQTargetPage(this);
    pluginPage = new PluginPage(this);

    // Add pages
    addPage(introPage);
    addPage(settingsPage);
    addPage(targetPage);
    addPage(pluginPage);

    // Set minimum size
    setMinimumSize(600, 550);
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
    QString parameters = targetPage->getParameters();
    bool extendedRedir = targetPage->getExtendedRedir();
    QStringList plugins = pluginPage->getSelectedPlugins();

    // Get target information based on mode
    QString targetInfo;
    const GameComponent* component = targetPage->getSelectedComponent();

    if (component) {
        // Mode 1: Supported Games (Registry-based)
        targetInfo = QString("Registry-Based Target (Supported Game):\n"
                           "  Component: %1\n"
                           "  File: %2\n"
                           "  Target File: %3\n"
                           "  Shunt Count: %4\n"
                           "  Extended Redir: %5")
                        .arg(component->componentName)
                        .arg(component->fileName)
                        .arg(component->targetFileName)
                        .arg(component->shuntCount)
                        .arg(component->extendedRedir ? "Yes" : "No");
    } else if (targetPage->isRegistryBased()) {
        // Mode 2: Custom Registry
        QString regKey = targetPage->getCustomRegistryKey();
        QString regValue = targetPage->getCustomRegistryValue();
        QString exeFile = targetPage->getCustomRegistryExe();
        QString targetFile = targetPage->getCustomRegistryTargetFile();
        int shuntCount = targetPage->getCustomRegistryShuntCount();
        bool isFullPath = targetPage->getCustomRegistryIsFullPath();
        uint32_t flags = targetPage->getCustomRegistryFlags();

        QStringList flagsList;
        if (flags & MPQD_EXTENDED_REDIR) flagsList << "Extended Redir";
        if (flags & MPQD_NO_SPAWNING) flagsList << "No Spawning";
        QString flagsStr = flagsList.isEmpty() ? "(none)" : flagsList.join(", ");

        targetInfo = QString("Registry-Based Target (Custom):\n"
                           "  Registry Key: %1\n"
                           "  Registry Value: %2\n"
                           "  Executable: %3\n"
                           "  Target File: %4\n"
                           "  Shunt Count: %5\n"
                           "  Value is Full Path: %6\n"
                           "  Flags: %7")
                        .arg(regKey)
                        .arg(regValue)
                        .arg(exeFile)
                        .arg(targetFile.isEmpty() ? "(same as executable)" : targetFile)
                        .arg(shuntCount)
                        .arg(isFullPath ? "Yes" : "No")
                        .arg(flagsStr);
    } else {
        // Mode 3: Custom Target (Hardcoded Path)
        QString customPath = targetPage->getCustomTargetPath();
        targetInfo = QString("Custom Target Path:\n  %1\n  Extended Redir: %2")
                        .arg(customPath)
                        .arg(extendedRedir ? "Yes" : "No");
    }

    // TODO: Call the actual SEMPQ creation code
    // For now, just show a message
    QString message = QString("SEMPQ Configuration:\n\n"
                             "Name: %1\n"
                             "Source MPQ: %2\n"
                             "Icon: %3\n\n"
                             "%4\n\n"
                             "Parameters: %5\n"
                             "Plugins: %6")
                        .arg(sempqName)
                        .arg(mpqPath)
                        .arg(iconPath.isEmpty() ? "(default)" : iconPath)
                        .arg(targetInfo)
                        .arg(parameters.isEmpty() ? "(none)" : parameters)
                        .arg(QString::number(plugins.count()));

    QMessageBox::information(this, "SEMPQ Ready",
                            message + "\n\nSEMPQ creation functionality will be implemented next.");
}
