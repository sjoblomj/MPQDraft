/*
    PatchWizard - Implementation
*/

#include "patchwizard.h"
#include "pluginpage.h"
#include "common/patcher.h"
#include "gamedata_qt.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QFileInfo>
#include <QDir>
#include <QIcon>
#include <QScrollArea>
#include <QFrame>
#include <QListWidget>

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
    setPixmap(QWizard::LogoPixmap, QPixmap(":/icons/mpqdraft.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

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

        "<p>Click <b>Next</b> to begin selecting your target game and MPQ files.</p>"
    );
    introLabel->setWordWrap(true);
    introLabel->setTextFormat(Qt::RichText);
    introLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(introLabel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    layout->addWidget(scrollArea);
}

//=============================================================================
// Page 1: Target Selection
//=============================================================================
TargetSelectionPage::TargetSelectionPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Select Target Executable");
    setSubTitle("Choose the game executable to patch and any command-line parameters.");
    setPixmap(QWizard::LogoPixmap, QPixmap(":/icons/blizzard/bnet.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Create tab widget
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    //=========================================================================
    // Tab 1: Detected Games
    //=========================================================================
    QWidget *detectedGamesTab = new QWidget(tabWidget);
    QVBoxLayout *detectedLayout = new QVBoxLayout(detectedGamesTab);

    QLabel *detectedLabel = new QLabel(
        "The following games have been detected on your computer, based on the "
        "Windows Registry. Select a game from the list below, and MPQDraft will "
        "automatically use the correct settings.",
        detectedGamesTab);
    detectedLabel->setWordWrap(true);
    detectedLayout->addWidget(detectedLabel);

    detectedLayout->addSpacing(10);

    gameList = new QListWidget(detectedGamesTab);
    gameList->setIconSize(QSize(32, 32));
    connect(gameList, &QListWidget::currentItemChanged, this, &TargetSelectionPage::onGameSelectionChanged);
    detectedLayout->addWidget(gameList);

    // Populate the list with detected games
    populateInstalledGames();

    tabWidget->addTab(detectedGamesTab, "Detected &Games");

    //=========================================================================
    // Tab 2: Custom Executable
    //=========================================================================
    QWidget *customExeTab = new QWidget(tabWidget);
    QVBoxLayout *customLayout = new QVBoxLayout(customExeTab);

    QLabel *customLabel = new QLabel(
        "Browse for an executable and configure patching options manually.",
        customExeTab);
    customLabel->setWordWrap(true);
    customLayout->addWidget(customLabel);

    customLayout->addSpacing(10);

    // Target path
    QLabel *targetLabel = new QLabel("<b>Executable Path:</b>", customExeTab);
    customLayout->addWidget(targetLabel);

    QHBoxLayout *targetLayout = new QHBoxLayout();
    customTargetPathEdit = new QLineEdit(customExeTab);
    customTargetPathEdit->setPlaceholderText("Path to game executable (e.g., StarCraft.exe)");
    customBrowseButton = new QPushButton("Bro&wse...", customExeTab);
    connect(customBrowseButton, &QPushButton::clicked, this, &TargetSelectionPage::onBrowseClicked);
    connect(customTargetPathEdit, &QLineEdit::textChanged, this, &TargetSelectionPage::onTargetPathChanged);
    targetLayout->addWidget(customTargetPathEdit);
    targetLayout->addWidget(customBrowseButton);
    customLayout->addLayout(targetLayout);

    customLayout->addSpacing(15);

    // Advanced Settings - collapsible section
    QPushButton *advancedToggle = new QPushButton("Advanced &Settings", customExeTab);
    advancedToggle->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    advancedToggle->setFlat(true);
    advancedToggle->setStyleSheet("QPushButton { text-align: left; font-weight: bold; }");
    advancedToggle->setCursor(Qt::PointingHandCursor);
    customLayout->addWidget(advancedToggle);

    advancedWidget = new QWidget(customExeTab);
    advancedWidget->setVisible(false);  // Hidden by default
    QVBoxLayout *advancedLayout = new QVBoxLayout(advancedWidget);
    advancedLayout->setContentsMargins(20, 0, 0, 0);  // Indent the content

    // Connect toggle button - capture pointer to widget
    QWidget *advancedWidgetPtr = advancedWidget;
    connect(advancedToggle, &QPushButton::clicked, [advancedToggle, advancedWidgetPtr, customExeTab]() {
        bool isVisible = advancedWidgetPtr->isVisible();
        advancedWidgetPtr->setVisible(!isVisible);
        advancedToggle->setIcon(customExeTab->style()->standardIcon(
            isVisible ? QStyle::SP_ArrowRight : QStyle::SP_ArrowDown));
    });

    // Parameters
    QLabel *paramsLabel = new QLabel("<b>Command-line Parameters (optional):</b>", advancedWidget);
    advancedLayout->addWidget(paramsLabel);
    customParametersEdit = new QLineEdit(advancedWidget);
    customParametersEdit->setPlaceholderText("e.g., -window -opengl");
    advancedLayout->addWidget(customParametersEdit);

    advancedLayout->addSpacing(10);

    // Extended redirect option with info icon
    QHBoxLayout *extendedRedirLayout = new QHBoxLayout();

    customExtendedRedirCheck = new QCheckBox("Use extended file redirection", advancedWidget);
    customExtendedRedirCheck->setChecked(true);  // Default to checked (most games need it)
    extendedRedirLayout->addWidget(customExtendedRedirCheck);

    // Info icon with detailed explanation
    QLabel *extendedRedirHelp = new QLabel(advancedWidget);
    extendedRedirHelp->setText(" ? ");
    extendedRedirHelp->setStyleSheet(
        "QLabel { "
        "background-color: #0079ff; "
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
    extendedRedirHelp->setToolTip(
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
    extendedRedirHelp->setCursor(Qt::WhatsThisCursor);
    extendedRedirLayout->addWidget(extendedRedirHelp);

    extendedRedirLayout->addStretch();
    advancedLayout->addLayout(extendedRedirLayout);

    advancedLayout->addSpacing(10);

    // Shunt Count
    QHBoxLayout *shuntCountLabelLayout = new QHBoxLayout();
    QLabel *shuntCountLabel = new QLabel("<b>Shunt Count:</b>", advancedWidget);
    shuntCountLabelLayout->addWidget(shuntCountLabel);

    QLabel *shuntCountHelp = new QLabel(advancedWidget);
    shuntCountHelp->setText(" ? ");
    shuntCountHelp->setStyleSheet(
        "QLabel { background-color: #0079ff; color: white; border-radius: 10px; "
        "font-weight: bold; font-size: 12px; padding: 2px; min-width: 16px; "
        "max-width: 16px; min-height: 16px; max-height: 16px; "
        "qproperty-alignment: AlignCenter; }");
    shuntCountHelp->setToolTip(
        "<b>Shunt Count</b><br><br>"
        "The number of times the game restarts itself before MPQDraft activates patching.<br><br>"
        "<b>0 (default):</b> Activate immediately when the game starts. Use this for most games.<br><br>"
        "<b>1:</b> Wait for the game to restart itself once before activating. Some games with "
        "copy protection (like Diablo) restart themselves after checking the CD, so MPQDraft "
        "needs to wait for this restart.<br><br>"
        "<b>Higher values:</b> Rarely needed, but available if a game restarts multiple times "
        "during its startup sequence.");
    shuntCountHelp->setCursor(Qt::WhatsThisCursor);
    shuntCountLabelLayout->addWidget(shuntCountHelp);
    shuntCountLabelLayout->addStretch();
    advancedLayout->addLayout(shuntCountLabelLayout);

    customShuntCountSpinBox = new QSpinBox(advancedWidget);
    customShuntCountSpinBox->setMinimum(0);
    customShuntCountSpinBox->setMaximum(INT_MAX);
    customShuntCountSpinBox->setValue(0);
    advancedLayout->addWidget(customShuntCountSpinBox);

    advancedLayout->addSpacing(10);

    // No Spawning flag
    QHBoxLayout *noSpawningLayout = new QHBoxLayout();

    customNoSpawningCheck = new QCheckBox("Do not inject into child processes", advancedWidget);
    customNoSpawningCheck->setChecked(false);  // Default to unchecked
    noSpawningLayout->addWidget(customNoSpawningCheck);

    QLabel *noSpawningHelp = new QLabel(advancedWidget);
    noSpawningHelp->setText(" ? ");
    noSpawningHelp->setStyleSheet(
        "QLabel { background-color: #0079ff; color: white; border-radius: 10px; "
        "font-weight: bold; font-size: 12px; padding: 2px; min-width: 16px; "
        "max-width: 16px; min-height: 16px; max-height: 16px; "
        "qproperty-alignment: AlignCenter; }");
    noSpawningHelp->setToolTip(
        "<b>Do Not Inject Into Child Processes (MPQD_NO_SPAWNING)</b><br><br>"
        "By default, MPQDraft injects itself into any child processes created by the game. "
        "This ensures that patches work even if the game launches additional executables.<br><br>"
        "<b>When to enable:</b> Some games launch helper processes (updaters, launchers, "
        "crash reporters) that don't need patching and may cause issues if MPQDraft injects "
        "into them. Enable this flag to prevent injection into child processes.<br><br>"
        "<b>When to disable (default):</b> Most games work fine with child process injection, "
        "and some games require it for patches to work correctly.");
    noSpawningHelp->setCursor(Qt::WhatsThisCursor);
    noSpawningLayout->addWidget(noSpawningHelp);

    noSpawningLayout->addStretch();
    advancedLayout->addLayout(noSpawningLayout);

    customLayout->addWidget(advancedWidget);

    customLayout->addStretch();

    tabWidget->addTab(customExeTab, "Custom &Executable");

    // Connect text change to completeChanged signal for validation
    connect(customTargetPathEdit, &QLineEdit::textChanged, this, &TargetSelectionPage::completeChanged);
    connect(tabWidget, &QTabWidget::currentChanged, this, &TargetSelectionPage::completeChanged);
}

bool TargetSelectionPage::isComplete() const
{
    int currentTab = tabWidget->currentIndex();

    if (currentTab == 0) {
        // Detected Games tab - complete if a game is selected
        return gameList->currentItem() != nullptr;
    } else {
        // Custom Executable tab - complete if target path is valid
        QString targetPath = customTargetPathEdit->text().trimmed();
        if (targetPath.isEmpty()) {
            return false;
        }

        // Check if file exists and is a valid file
        QFileInfo fileInfo(targetPath);
        return fileInfo.exists() && fileInfo.isFile();
    }
}

QString TargetSelectionPage::getTargetPath() const
{
    int currentTab = tabWidget->currentIndex();

    if (currentTab == 0) {
        // Detected Games tab - get path from selected game
        QListWidgetItem *current = gameList->currentItem();
        if (current) {
            return current->data(Qt::UserRole + 1).toString();
        }
        return QString();
    } else {
        // Custom Executable tab
        return customTargetPathEdit->text();
    }
}

QString TargetSelectionPage::getParameters() const
{
    int currentTab = tabWidget->currentIndex();

    if (currentTab == 0) {
        // Detected Games tab - no parameters (could be extended in future)
        return QString();
    } else {
        // Custom Executable tab
        return customParametersEdit->text();
    }
}

bool TargetSelectionPage::useExtendedRedir() const
{
    int currentTab = tabWidget->currentIndex();

    if (currentTab == 0) {
        // Detected Games tab - get from selected game's data
        QListWidgetItem *current = gameList->currentItem();
        if (current) {
            return current->data(Qt::UserRole).toBool();
        }
        return true;  // Default to true
    } else {
        // Custom Executable tab
        return customExtendedRedirCheck->isChecked();
    }
}

int TargetSelectionPage::getShuntCount() const
{
    int currentTab = tabWidget->currentIndex();

    if (currentTab == 0) {
        // Detected Games tab - shunt count is stored in the game data
        // We need to retrieve it from the component
        // For now, return 0 as detected games have their shunt count in the game data
        // This will be handled by the wizard when it looks up the component
        return 0;
    } else {
        // Custom Executable tab
        return customShuntCountSpinBox->value();
    }
}

bool TargetSelectionPage::useNoSpawning() const
{
    int currentTab = tabWidget->currentIndex();

    if (currentTab == 0) {
        // Detected Games tab - get from game data flags
        // For now, return false as this is handled by the component's flags
        return false;
    } else {
        // Custom Executable tab
        return customNoSpawningCheck->isChecked();
    }
}

void TargetSelectionPage::validateTargetPath()
{
    QString targetPath = customTargetPathEdit->text().trimmed();

    if (targetPath.isEmpty()) {
        customTargetPathEdit->setStyleSheet("");
        customTargetPathEdit->setToolTip("");
        return;
    }

    QFileInfo fileInfo(targetPath);
    if (!fileInfo.exists()) {
        customTargetPathEdit->setStyleSheet(INVALID_FIELD_STYLE);
        customTargetPathEdit->setToolTip("File does not exist");
    } else if (!fileInfo.isFile()) {
        customTargetPathEdit->setStyleSheet(INVALID_FIELD_STYLE);
        customTargetPathEdit->setToolTip("Path is not a file");
    } else {
        customTargetPathEdit->setStyleSheet("");
        customTargetPathEdit->setToolTip("");
    }
}

void TargetSelectionPage::onTargetPathChanged()
{
    validateTargetPath();
}

void TargetSelectionPage::populateInstalledGames()
{
    // Get list of installed games
    QVector<const SupportedGame*> installedGames = getInstalledGamesQt();

    if (installedGames.isEmpty()) {
        // No games detected - show a message
        QListWidgetItem *item = new QListWidgetItem(gameList);
        item->setText("No supported games detected");
        item->setIcon(QIcon(":/icons/not-found.png"));
        item->setFlags(Qt::NoItemFlags);  // Make it non-selectable
        item->setForeground(QBrush(QColor(128, 128, 128)));  // Gray text
        return;
    }

    // Populate game list with detected games and their components
    for (const SupportedGame* game : installedGames) {
        for (const GameComponent& component : game->components) {
            // Verify that the component actually exists on disk
            QString componentPath = locateComponentQt(getRegistryKey(*game), getRegistryValue(*game), getFileName(component));
            if (componentPath.isEmpty()) {
                continue;  // Skip components that don't exist
            }

            QListWidgetItem *item = new QListWidgetItem(gameList);

            // Display format: "Game Name - Component Name" or just "Game Name" for single component
            QString displayText;
            if (game->components.size() == 1) {
                displayText = getGameName(*game);
            } else {
                displayText = QString("%1 - %2").arg(getGameName(*game), getComponentName(component));
            }
            item->setText(displayText);

            // Set icon
            QIcon icon(getIconPath(component));
            item->setIcon(icon);

            // Store the extended redir flag in UserRole
            item->setData(Qt::UserRole, component.extendedRedir);

            // Store the full path in UserRole+1
            item->setData(Qt::UserRole + 1, componentPath);
        }
    }
}

void TargetSelectionPage::onGameSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);

    if (!current) {
        return;
    }

    // Emit completeChanged to update wizard buttons
    emit completeChanged();
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
        customTargetPathEdit->setText(fileName);

        // Set focus based on whether Advanced Settings is expanded
        if (advancedWidget->isVisible()) {
            customParametersEdit->setFocus();
        } else {
            customTargetPathEdit->setFocus();
        }
    }
}

//=============================================================================
// Page 2: MPQ Selection
//=============================================================================
MPQSelectionPage::MPQSelectionPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Select MPQ Files");
    setSubTitle("Add MPQ files to load. Files are loaded in order (files higher up have higher priority).");
    setPixmap(QWizard::LogoPixmap, QPixmap(":/icons/mpq.svg").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

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
    mpqListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mpqListWidget->setDragDropMode(QAbstractItemView::InternalMove);
    mpqListWidget->setDefaultDropAction(Qt::MoveAction);
    connect(mpqListWidget, &QListWidget::itemChanged, this, &MPQSelectionPage::onItemChanged);
    connect(mpqListWidget, &QListWidget::itemSelectionChanged, this, &MPQSelectionPage::onSelectionChanged);
    contentLayout->addWidget(mpqListWidget);

    // Buttons
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    addButton = new QPushButton("Add &MPQ...", this);
    addFolderButton = new QPushButton("Add &Folder...", this);
    removeButton = new QPushButton("&Remove", this);
    moveUpButton = new QPushButton("Move &Up", this);
    moveDownButton = new QPushButton("Move &Down", this);

    // Initially disable buttons that require selection
    removeButton->setEnabled(false);
    moveUpButton->setEnabled(false);
    moveDownButton->setEnabled(false);

    connect(addButton, &QPushButton::clicked, this, &MPQSelectionPage::onAddClicked);
    connect(addFolderButton, &QPushButton::clicked, this, &MPQSelectionPage::onAddFolderClicked);
    connect(removeButton, &QPushButton::clicked, this, &MPQSelectionPage::onRemoveClicked);
    connect(moveUpButton, &QPushButton::clicked, this, &MPQSelectionPage::onMoveUpClicked);
    connect(moveDownButton, &QPushButton::clicked, this, &MPQSelectionPage::onMoveDownClicked);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(addFolderButton);
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
        QListWidgetItem *item = mpqListWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            mpqs.append(item->text());
        }
    }
    return mpqs;
}

bool MPQSelectionPage::isComplete() const
{
    // Count checked MPQs
    int checkedCount = 0;
    for (int i = 0; i < mpqListWidget->count(); ++i) {
        if (mpqListWidget->item(i)->checkState() == Qt::Checked) {
            checkedCount++;
        }
    }

    // Page is complete if we don't have too many MPQs selected
    return checkedCount <= MAX_PATCH_MPQS;
}

bool MPQSelectionPage::validatePage()
{
    // Count checked MPQs
    int checkedCount = 0;
    for (int i = 0; i < mpqListWidget->count(); ++i) {
        if (mpqListWidget->item(i)->checkState() == Qt::Checked) {
            checkedCount++;
        }
    }

    // If there are items but none are checked, warn the user
    if (mpqListWidget->count() > 0 && checkedCount == 0) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "No MPQs Selected",
            "You have MPQ files in the list, but none are selected (checked). "
            "Do you want to proceed without any MPQ files?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

        return reply == QMessageBox::Yes;
    }

    return true;
}

void MPQSelectionPage::validateMPQList()
{
    int mpqCount = mpqListWidget->count();
    int checkedCount = 0;

    // Count checked MPQs and check for missing files
    QStringList missingFiles;
    for (int i = 0; i < mpqCount; ++i) {
        QListWidgetItem *item = mpqListWidget->item(i);
        QString mpqPath = item->text();
        QFileInfo fileInfo(mpqPath);

        if (item->checkState() == Qt::Checked) {
            checkedCount++;
        }

        if (!fileInfo.exists()) {
            missingFiles.append(QFileInfo(mpqPath).fileName());
            item->setForeground(QColor("#d32f2f"));
            item->setToolTip("File does not exist");
        } else {
            item->setForeground(QColor());
            item->setToolTip("");
        }
    }

    // Check if too many checked MPQs
    if (checkedCount > MAX_PATCH_MPQS) {
        statusLabel->setText(QString("<font color='#d32f2f'><b>Warning:</b> Too many MPQ files selected (%1/%2). "
                                     "Please uncheck some files.</font>")
                            .arg(checkedCount).arg(MAX_PATCH_MPQS));
        statusLabel->show();
        return;
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
    emit completeChanged();
}

void MPQSelectionPage::addMPQFile(const QString &fileName, bool checked)
{
    // Don't add duplicates - silently filter them out
    for (int i = 0; i < mpqListWidget->count(); ++i) {
        if (mpqListWidget->item(i)->text() == fileName) {
            return;  // Already in the list
        }
    }

    QListWidgetItem *item = new QListWidgetItem(fileName);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);

    // Add MPQ icon
    QIcon mpqIcon(":/icons/mpq.svg");
    item->setIcon(mpqIcon);

    mpqListWidget->addItem(item);
}

void MPQSelectionPage::onAddClicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        "Select MPQ Files",
        QString(),
        "MPQ Archives (*.mpq);;All Files (*.*)"
    );

    // Clear selection before adding new items
    mpqListWidget->clearSelection();

    for (const QString &fileName : fileNames) {
        int oldCount = mpqListWidget->count();
        addMPQFile(fileName, true);  // Add as checked

        // Select the newly added item (if it was actually added)
        if (mpqListWidget->count() > oldCount) {
            mpqListWidget->item(mpqListWidget->count() - 1)->setSelected(true);
        }
    }

    validateMPQList();
}

void MPQSelectionPage::onAddFolderClicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "Select Folder Containing MPQ Files",
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (folderPath.isEmpty()) {
        return;  // User cancelled
    }

    // Find all MPQ files in the selected folder
    QDir dir(folderPath);
    QStringList mpqFiles = dir.entryList(QStringList() << "*.mpq" << "*.MPQ", QDir::Files);

    // Clear selection before adding new items
    mpqListWidget->clearSelection();

    for (const QString &fileName : mpqFiles) {
        QString fullPath = dir.absoluteFilePath(fileName);
        int oldCount = mpqListWidget->count();
        addMPQFile(fullPath, false);  // Add as unchecked

        // Select the newly added item (if it was actually added)
        if (mpqListWidget->count() > oldCount) {
            mpqListWidget->item(mpqListWidget->count() - 1)->setSelected(true);
        }
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

void MPQSelectionPage::onSelectionChanged()
{
    int selectedCount = mpqListWidget->selectedItems().count();

    // Remove button: enabled if 1 or more items selected
    removeButton->setEnabled(selectedCount >= 1);

    // Move Up/Down buttons: enabled only if exactly 1 item selected
    moveUpButton->setEnabled(selectedCount == 1);
    moveDownButton->setEnabled(selectedCount == 1);
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

    // Enable minimize and maximize buttons - use Window flag instead of Dialog
    setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

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
    int shuntCount = targetPage->getShuntCount();
    bool noSpawning = targetPage->useNoSpawning();
    QStringList mpqs = mpqPage->getSelectedMPQs();
    QStringList plugins = pluginPage->getSelectedPlugins();

    // TODO: Call the actual patcher DLL
    // For now, just show a message
    QString message = QString("Patch Configuration:\n\n"
                             "Target: %1\n"
                             "Parameters: %2\n"
                             "Extended Redir: %3\n"
                             "Shunt Count: %4\n"
                             "No Spawning: %5\n"
                             "MPQs: %6\n"
                             "Plugins: %7")
                        .arg(targetPath)
                        .arg(parameters.isEmpty() ? "(none)" : parameters)
                        .arg(extendedRedir ? "Yes" : "No")
                        .arg(shuntCount)
                        .arg(noSpawning ? "Yes" : "No")
                        .arg(QString::number(mpqs.count()))
                        .arg(QString::number(plugins.count()));

    QMessageBox::information(this, "Patch Ready",
                            message + "\n\nPatching functionality will be implemented next.");
}
