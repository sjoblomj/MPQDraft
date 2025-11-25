/*
    PatchWizard - Implementation
*/

#include "patchwizard.h"
#include "pluginpage.h"
#include "common/patcher.h"
#include "core/gamedata.h"
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
#include <QDebug>
#include <QSettings>
#include <QComboBox>
#include <QScrollBar>
#include <QMenu>

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
    gameList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(gameList, &QListWidget::currentItemChanged, this, &TargetSelectionPage::onGameSelectionChanged);
    connect(gameList, &QListWidget::customContextMenuRequested, this, &TargetSelectionPage::onGameListContextMenu);
    detectedLayout->addWidget(gameList);

    // Populate the list with detected games
    populateInstalledGames();

    tabWidget->addTab(detectedGamesTab, "Detected &Games");

    //=========================================================================
    // Tab 2: Custom Executable
    //=========================================================================
    QWidget *customExeTab = new QWidget(tabWidget);
    QVBoxLayout *customTabLayout = new QVBoxLayout(customExeTab);
    customTabLayout->setContentsMargins(0, 0, 0, 0);

    // Add scroll area
    QScrollArea *customScrollArea = new QScrollArea(customExeTab);
    customScrollArea->setWidgetResizable(true);
    customScrollArea->setFrameShape(QFrame::NoFrame);
    customScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Set background to match tab background (white)
    QPalette scrollPalette = customScrollArea->palette();
    scrollPalette.setColor(QPalette::Window, Qt::white);
    customScrollArea->setPalette(scrollPalette);
    customScrollArea->setAutoFillBackground(true);

    customTabLayout->addWidget(customScrollArea);

    QWidget *customScrollWidget = new QWidget();
    customScrollArea->setWidget(customScrollWidget);
    QVBoxLayout *customLayout = new QVBoxLayout(customScrollWidget);

    QLabel *customLabel = new QLabel(
        "Browse for an executable and configure patching options manually.",
        customScrollWidget);
    customLabel->setWordWrap(true);
    customLayout->addWidget(customLabel);

    customLayout->addSpacing(10);

    // Target path
    QLabel *targetLabel = new QLabel("<b>Executable Path:</b>", customScrollWidget);
    customLayout->addWidget(targetLabel);

    QHBoxLayout *targetLayout = new QHBoxLayout();
    customTargetPathEdit = new QLineEdit(customScrollWidget);
    customTargetPathEdit->setPlaceholderText("Path to game executable (e.g., StarCraft.exe)");
    customBrowseButton = new QPushButton("Bro&wse...", customScrollWidget);
    connect(customBrowseButton, &QPushButton::clicked, this, &TargetSelectionPage::onBrowseClicked);
    connect(customTargetPathEdit, &QLineEdit::textChanged, this, &TargetSelectionPage::onTargetPathChanged);
    targetLayout->addWidget(customTargetPathEdit);
    targetLayout->addWidget(customBrowseButton);
    customLayout->addLayout(targetLayout);

    customLayout->addSpacing(15);

    // Advanced Settings - collapsible section
    QPushButton *advancedToggle = new QPushButton("Advanced &Settings", customScrollWidget);
    advancedToggle->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    advancedToggle->setFlat(true);
    advancedToggle->setStyleSheet("QPushButton { text-align: left; font-weight: bold; }");
    advancedToggle->setCursor(Qt::PointingHandCursor);
    customLayout->addWidget(advancedToggle);

    advancedWidget = new QWidget(customScrollWidget);
    advancedWidget->setVisible(false);  // Hidden by default
    QVBoxLayout *advancedLayout = new QVBoxLayout(advancedWidget);
    advancedLayout->setContentsMargins(20, 0, 0, 0);  // Indent the content

    // Connect toggle button - capture pointer to widget
    QWidget *advancedWidgetPtr = advancedWidget;
    connect(advancedToggle, &QPushButton::clicked, [advancedToggle, advancedWidgetPtr, customScrollWidget]() {
        bool isVisible = advancedWidgetPtr->isVisible();
        advancedWidgetPtr->setVisible(!isVisible);
        advancedToggle->setIcon(customScrollWidget->style()->standardIcon(
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

    customLayout->addSpacing(15);

    // Remember Application - collapsible section
    QPushButton *rememberAppToggle = new QPushButton("&Remember Application", customScrollWidget);
    rememberAppToggle->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    rememberAppToggle->setFlat(true);
    rememberAppToggle->setStyleSheet("QPushButton { text-align: left; font-weight: bold; }");
    rememberAppToggle->setCursor(Qt::PointingHandCursor);
    customLayout->addWidget(rememberAppToggle);

    QWidget *rememberAppWidget = new QWidget(customScrollWidget);
    rememberAppWidget->setVisible(false);  // Hidden by default
    QVBoxLayout *rememberAppLayout = new QVBoxLayout(rememberAppWidget);
    rememberAppLayout->setContentsMargins(20, 0, 0, 0);  // Indent the content

    // Connect toggle button
    connect(rememberAppToggle, &QPushButton::clicked, [rememberAppToggle, rememberAppWidget, customScrollWidget]() {
        bool isVisible = rememberAppWidget->isVisible();
        rememberAppWidget->setVisible(!isVisible);
        rememberAppToggle->setIcon(customScrollWidget->style()->standardIcon(
            isVisible ? QStyle::SP_ArrowRight : QStyle::SP_ArrowDown));
    });

    // Information box with icon
    QWidget *infoWidget = new QWidget(rememberAppWidget);
    infoWidget->setStyleSheet(
        "QWidget { "
        "background-color: #d1ecf1; "
        "border: 1px solid #17a2b8; "
        "border-radius: 4px; "
        "}");
    QHBoxLayout *infoLayout = new QHBoxLayout(infoWidget);
    infoLayout->setContentsMargins(10, 10, 10, 10);
    infoLayout->setSpacing(8);

    // Info icon (save-to-list.svg scaled to 64x64)
    QLabel *infoIcon = new QLabel(infoWidget);
    QPixmap infoPixmap(":/icons/save-to-list.svg");
    infoIcon->setPixmap(infoPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    infoIcon->setAlignment(Qt::AlignTop);
    infoIcon->setStyleSheet("QLabel { background-color: transparent; border: none; }");
    infoIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    infoLayout->addWidget(infoIcon);

    // Info text
    QLabel *rememberAppLabel = new QLabel(
            "Save the above custom executable configuration to make it appear in the Detected Games list. "
            "You can override settings for pre-configured games or add entirely new applications. "
            "The data is saved to the Windows Registry.<br><br>"
            "Choose an existing game from the dropdown to override its settings, or type a new name to add "
            "a custom application.", infoWidget);
    rememberAppLabel->setWordWrap(true);
    rememberAppLabel->setStyleSheet("QLabel { color: #0c5460; background-color: transparent; border: none; }");
    infoLayout->addWidget(rememberAppLabel, 1);  // Stretch factor of 1 to take remaining space

    rememberAppLayout->addWidget(infoWidget);

    rememberAppLayout->addSpacing(10);

    // Horizontal layout for dropdown and image
    QHBoxLayout *dropdownImageLayout = new QHBoxLayout();

    // Left side: Application Name dropdown
    QVBoxLayout *dropdownLayout = new QVBoxLayout();
    QLabel *appNameLabel = new QLabel("Application Name:", rememberAppWidget);
    dropdownLayout->addWidget(appNameLabel);

    QComboBox *appNameCombo = new QComboBox(rememberAppWidget);
    appNameCombo->setEditable(true);
    appNameCombo->setInsertPolicy(QComboBox::NoInsert);

    // Populate with all game components from gamedata
    std::vector<SupportedGame> games = getSupportedGames();
    for (const auto &game : games) {
        for (const auto &component : game.components) {
            QString displayName;
            // Display format: "Game Name - Component Name" or just "Game Name" for single component
            if (game.components.size() == 1) {
                displayName = QString::fromStdString(game.gameName);
            } else {
                displayName = QString::fromStdString(game.gameName) + " - " + QString::fromStdString(component.componentName);
            }
            appNameCombo->addItem(displayName);
        }
    }

    dropdownLayout->addWidget(appNameCombo);
    dropdownImageLayout->addLayout(dropdownLayout);

    dropdownImageLayout->addSpacing(10);

    // Right side: Image label (64x64)
    QLabel *rememberAppImageLabel = new QLabel(rememberAppWidget);
    rememberAppImageLabel->setFixedSize(64, 64);
    rememberAppImageLabel->setAlignment(Qt::AlignCenter);
    rememberAppImageLabel->setStyleSheet("QLabel { border: 1px solid #ccc; }");
    dropdownImageLayout->addWidget(rememberAppImageLabel);

    rememberAppLayout->addLayout(dropdownImageLayout);

    rememberAppLayout->addSpacing(10);

    // Icon path input (hidden by default, but takes up space)
    QLabel *iconPathLabel = new QLabel("Application icon:", rememberAppWidget);
    rememberAppLayout->addWidget(iconPathLabel);

    QHBoxLayout *iconPathLayout = new QHBoxLayout();
    QLineEdit *iconPathEdit = new QLineEdit(rememberAppWidget);
    iconPathEdit->setPlaceholderText("Path to icon file (optional)");
    QPushButton *browseIconButton = new QPushButton("Browse &Icon...", rememberAppWidget);
    iconPathLayout->addWidget(iconPathEdit);
    iconPathLayout->addWidget(browseIconButton);
    rememberAppLayout->addLayout(iconPathLayout);

    // Hide icon controls by default but keep their space in layout
    iconPathLabel->hide();
    iconPathEdit->hide();
    browseIconButton->hide();

    // Set size policy to maintain space when hidden
    QSizePolicy spLabel = iconPathLabel->sizePolicy();
    spLabel.setRetainSizeWhenHidden(true);
    iconPathLabel->setSizePolicy(spLabel);

    QSizePolicy spEdit = iconPathEdit->sizePolicy();
    spEdit.setRetainSizeWhenHidden(true);
    iconPathEdit->setSizePolicy(spEdit);

    QSizePolicy spButton = browseIconButton->sizePolicy();
    spButton.setRetainSizeWhenHidden(true);
    browseIconButton->setSizePolicy(spButton);

    rememberAppLayout->addSpacing(10);

    // Save button (taller)
    QPushButton *saveAppButton = new QPushButton("Save", rememberAppWidget);
    saveAppButton->setMinimumHeight(40);
    saveAppButton->setEnabled(false);  // Start disabled
    rememberAppLayout->addWidget(saveAppButton);

    // Browse icon button
    connect(browseIconButton, &QPushButton::clicked, [iconPathEdit, this]() {
        QString fileName = QFileDialog::getOpenFileName(
            this,
            "Select Icon",
            QString(),
            "Images (*.png *.jpg *.jpeg *.bmp *.gif *.svg *.ico);;All Files (*.*)"
        );
        if (!fileName.isEmpty()) {
            iconPathEdit->setText(fileName);
        }
    });

    // Validate both executable path and icon path
    auto validateSaveButton = [this, iconPathEdit, rememberAppImageLabel, saveAppButton, appNameCombo]() {
        QString appName = appNameCombo->currentText().trimmed();
        if (appName.isEmpty()) {
            saveAppButton->setEnabled(false);
            return;
        }

        // Check if this is a pre-configured game
        const auto &games = getSupportedGames();
        bool isPreConfigured = false;
        for (const auto &game : games) {
            for (const auto &component : game.components) {
                QString displayName;
                if (game.components.size() == 1) {
                    displayName = QString::fromStdString(game.gameName);
                } else {
                    displayName = QString::fromStdString(game.gameName) + " - " + QString::fromStdString(component.componentName);
                }
                if (displayName == appName) {
                    isPreConfigured = true;
                    break;
                }
            }
            if (isPreConfigured) break;
        }

        // For pre-configured games, we don't need to validate the executable path
        // (it will use the default path from gamedata)
        bool executableValid = isPreConfigured;
        if (!isPreConfigured) {
            // For custom applications, check executable path
            QString executablePath = customTargetPathEdit->text().trimmed();
            if (!executablePath.isEmpty()) {
                QFileInfo execFileInfo(executablePath);
                executableValid = execFileInfo.exists() && execFileInfo.isFile();
            }
        }

        // Check icon path (only relevant for custom applications)
        QString iconPath = iconPathEdit->text().trimmed();
        bool iconValid = true;  // Empty is valid
        if (!iconPath.isEmpty()) {
            QFileInfo iconFileInfo(iconPath);
            if (!iconFileInfo.exists() || !iconFileInfo.isFile()) {
                iconValid = false;
                iconPathEdit->setStyleSheet("QLineEdit { border: 2px solid #ff6b6b; background-color: #ffe0e0; }");
            } else {
                iconPathEdit->setStyleSheet("");
                // Try to load as image
                QPixmap pixmap(iconPath);
                if (!pixmap.isNull()) {
                    rememberAppImageLabel->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            }
        } else {
            iconPathEdit->setStyleSheet("");
        }

        // Enable save button if executable is valid (or pre-configured) and icon is valid
        saveAppButton->setEnabled(executableValid && iconValid);
    };

    connect(iconPathEdit, &QLineEdit::textChanged, validateSaveButton);
    connect(customTargetPathEdit, &QLineEdit::textChanged, validateSaveButton);
    connect(appNameCombo, &QComboBox::currentTextChanged, validateSaveButton);

    // Helper lambda to generate display name for a component
    auto getDisplayName = [](const SupportedGame &game, const GameComponent &component) -> QString {
        // Display format: "Game Name - Component Name" or just "Game Name" for single component
        if (game.components.size() == 1) {
            return QString::fromStdString(game.gameName);
        } else {
            return QString::fromStdString(game.gameName) + " - " + QString::fromStdString(component.componentName);
        }
    };

    // Update image and visibility when dropdown changes
    connect(appNameCombo, &QComboBox::currentTextChanged, [games, getDisplayName, iconPathLabel, iconPathEdit, browseIconButton, validateSaveButton, rememberAppImageLabel](const QString &text) {
        // Find component by display name and load its icon
        bool isPreConfigured = false;
        for (const auto &game : games) {
            for (const auto &component : game.components) {
                if (getDisplayName(game, component) == text) {
                    QPixmap pixmap(QString::fromStdString(component.iconPath));
                    if (!pixmap.isNull()) {
                        rememberAppImageLabel->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    }
                    isPreConfigured = true;
                    break;
                }
            }
            if (isPreConfigured) break;
        }

        if (isPreConfigured) {
            // Hide icon controls and clear icon path
            iconPathLabel->hide();
            iconPathEdit->hide();
            browseIconButton->hide();
            iconPathEdit->setText("");
        } else {
            // Show icon controls for custom application
            iconPathLabel->show();
            iconPathEdit->show();
            browseIconButton->show();
            // Clear the image if no custom icon is set
            if (iconPathEdit->text().trimmed().isEmpty()) {
                rememberAppImageLabel->clear();
            } else {
                validateSaveButton();
            }
        }
    });

    // Set initial image if there's a default selection
    if (appNameCombo->count() > 0) {
        QString initialText = appNameCombo->currentText();
        for (const auto &game : games) {
            for (const auto &component : game.components) {
                if (getDisplayName(game, component) == initialText) {
                    QPixmap pixmap(QString::fromStdString(component.iconPath));
                    if (!pixmap.isNull()) {
                        rememberAppImageLabel->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    }
                    break;
                }
            }
        }
        // Run initial validation to enable Save button if appropriate
        validateSaveButton();
    }

    // Save button action
    connect(saveAppButton, &QPushButton::clicked, [this, appNameCombo, iconPathEdit]() {
        QString appName = appNameCombo->currentText().trimmed();
        if (appName.isEmpty()) {
            return;
        }

        // Get values from Custom Executable tab
        QString executablePath = customTargetPathEdit->text().trimmed();
        QString parameters = customParametersEdit->text();
        int shuntCount = customShuntCountSpinBox->value();
        bool extendedRedir = customExtendedRedirCheck->isChecked();
        bool noSpawning = customNoSpawningCheck->isChecked();
        QString iconPath = iconPathEdit->text().trimmed();

        // Save to QSettings
        QSettings settings;
        settings.beginGroup("CustomApplications");
        settings.beginGroup(appName);
        settings.setValue("executablePath", executablePath);
        settings.setValue("parameters", parameters);
        settings.setValue("shuntCount", shuntCount);
        settings.setValue("extendedRedir", extendedRedir);
        settings.setValue("noSpawning", noSpawning);
        if (!iconPath.isEmpty()) {
            settings.setValue("iconPath", iconPath);
        } else {
            settings.remove("iconPath");
        }
        settings.endGroup();
        settings.endGroup();

        // Refresh the game list to show the new/updated application
        populateInstalledGames();

        // Show success message
        QMessageBox::information(this, "Application Saved",
            QString("'%1' has been saved successfully and added to the Detected Games list.").arg(appName));
    });

    customLayout->addWidget(rememberAppWidget);

    customLayout->addStretch();

    tabWidget->addTab(customExeTab, "Custom &Executable");

    // Connect text change to completeChanged signal for validation
    connect(customTargetPathEdit, &QLineEdit::textChanged, this, &TargetSelectionPage::completeChanged);
    connect(tabWidget, &QTabWidget::currentChanged, this, &TargetSelectionPage::completeChanged);

    // Connect field changes to save settings
    connect(gameList, &QListWidget::currentRowChanged, this, &TargetSelectionPage::saveSettings);
    connect(tabWidget, &QTabWidget::currentChanged, this, &TargetSelectionPage::saveSettings);
    connect(customTargetPathEdit, &QLineEdit::textChanged, this, &TargetSelectionPage::saveSettings);
    connect(customParametersEdit, &QLineEdit::textChanged, this, &TargetSelectionPage::saveSettings);
    connect(customExtendedRedirCheck, &QCheckBox::stateChanged, this, &TargetSelectionPage::saveSettings);
    connect(customShuntCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TargetSelectionPage::saveSettings);
    connect(customNoSpawningCheck, &QCheckBox::stateChanged, this, &TargetSelectionPage::saveSettings);
}

bool TargetSelectionPage::isComplete() const
{
    int currentTab = tabWidget->currentIndex();

    if (currentTab == 0) {
        // Detected Games tab - complete if a game is selected and path is valid
        QListWidgetItem *current = gameList->currentItem();
        if (!current) {
            return false;
        }

        // Check if the executable path is valid
        QString executablePath = current->data(Qt::UserRole + 1).toString();
        QFileInfo fileInfo(executablePath);
        return fileInfo.exists() && fileInfo.isFile();
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

void TargetSelectionPage::initializePage()
{
    loadSettings();
    QWizardPage::initializePage();
}

void TargetSelectionPage::cleanupPage()
{
    QWizardPage::cleanupPage();
}

void TargetSelectionPage::saveSettings()
{
    QSettings settings;
    settings.beginGroup("PatchWizard/Target");

    // Save current tab
    settings.setValue("currentTab", tabWidget->currentIndex());

    // Save custom executable settings
    settings.setValue("customTargetPath", customTargetPathEdit->text());
    settings.setValue("customParameters", customParametersEdit->text());
    settings.setValue("customExtendedRedir", customExtendedRedirCheck->isChecked());
    settings.setValue("customShuntCount", customShuntCountSpinBox->value());
    settings.setValue("customNoSpawning", customNoSpawningCheck->isChecked());

    // Save selected game (if any)
    QListWidgetItem *currentItem = gameList->currentItem();
    if (currentItem) {
        settings.setValue("selectedGame", currentItem->text());
    }

    settings.endGroup();
}

void TargetSelectionPage::loadSettings()
{
    QSettings settings;
    settings.beginGroup("PatchWizard/Target");

    // Block signals while loading to avoid triggering saves
    tabWidget->blockSignals(true);
    customTargetPathEdit->blockSignals(true);
    customParametersEdit->blockSignals(true);
    customExtendedRedirCheck->blockSignals(true);
    customShuntCountSpinBox->blockSignals(true);
    customNoSpawningCheck->blockSignals(true);
    gameList->blockSignals(true);

    // Restore tab
    int savedTab = settings.value("currentTab", 0).toInt();
    tabWidget->setCurrentIndex(savedTab);

    // Restore custom executable settings
    customTargetPathEdit->setText(settings.value("customTargetPath", "").toString());
    customParametersEdit->setText(settings.value("customParameters", "").toString());
    customExtendedRedirCheck->setChecked(settings.value("customExtendedRedir", true).toBool());
    customShuntCountSpinBox->setValue(settings.value("customShuntCount", 0).toInt());
    customNoSpawningCheck->setChecked(settings.value("customNoSpawning", false).toBool());

    // Restore selected game
    QString selectedGame = settings.value("selectedGame", "").toString();
    if (!selectedGame.isEmpty()) {
        for (int i = 0; i < gameList->count(); ++i) {
            if (gameList->item(i)->text() == selectedGame) {
                gameList->setCurrentRow(i);
                break;
            }
        }
    }

    // Unblock signals
    tabWidget->blockSignals(false);
    customTargetPathEdit->blockSignals(false);
    customParametersEdit->blockSignals(false);
    customExtendedRedirCheck->blockSignals(false);
    customShuntCountSpinBox->blockSignals(false);
    customNoSpawningCheck->blockSignals(false);
    gameList->blockSignals(false);

    settings.endGroup();

    // Manually trigger validation to update the page's complete state
    // This is necessary because we blocked signals during loading
    validateTargetPath();
    emit completeChanged();
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
        // Detected Games tab - get parameters from selected item
        QListWidgetItem *currentItem = gameList->currentItem();
        if (currentItem) {
            return currentItem->data(Qt::UserRole + 5).toString();
        }
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
        // Detected Games tab - get from stored data
        QListWidgetItem *current = gameList->currentItem();
        if (current) {
            return current->data(Qt::UserRole + 2).toInt();
        }
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
        // Detected Games tab - get from stored data
        QListWidgetItem *current = gameList->currentItem();
        if (current) {
            return current->data(Qt::UserRole + 3).toBool();
        }
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
    gameList->clear();

    // Get list of installed games
    QVector<const SupportedGame*> installedGames = getInstalledGamesQt();

    // Load custom applications from QSettings
    QSettings settings;
    settings.beginGroup("CustomApplications");
    QStringList customAppNames = settings.childGroups();
    settings.endGroup();

    // Track which custom apps have been processed (to handle overrides)
    QSet<QString> processedCustomApps;

    // Populate game list with detected games and their components
    for (const SupportedGame* game : installedGames) {
        for (const GameComponent& component : game->components) {
            // Generate display name
            QString displayText;
            if (game->components.size() == 1) {
                displayText = getGameName(*game);
            } else {
                displayText = QString("%1 - %2").arg(getGameName(*game), getComponentName(component));
            }

            // Check if there's an override in QSettings
            bool hasOverride = customAppNames.contains(displayText);
            if (hasOverride) {
                processedCustomApps.insert(displayText);
            }

            QString componentPath;
            QString iconPath;
            QString parameters;
            bool extendedRedir;
            int shuntCount;
            bool noSpawning;
            bool isModified = false;
            bool pathInvalid = false;

            if (hasOverride) {
                // Load from QSettings
                settings.beginGroup("CustomApplications");
                settings.beginGroup(displayText);
                componentPath = settings.value("executablePath").toString();
                iconPath = settings.value("iconPath").toString();
                parameters = settings.value("parameters").toString();
                extendedRedir = settings.value("extendedRedir", component.extendedRedir).toBool();
                shuntCount = settings.value("shuntCount", component.shuntCount).toInt();
                noSpawning = settings.value("noSpawning", (component.flags & MPQD_NO_SPAWNING) != 0).toBool();
                settings.endGroup();
                settings.endGroup();

                isModified = true;

                // Verify path exists
                QFileInfo fileInfo(componentPath);
                if (!fileInfo.exists() || !fileInfo.isFile()) {
                    pathInvalid = true;
                }
            } else {
                // Use default values from gamedata
                componentPath = locateComponentQt(getRegistryKey(*game), getRegistryValue(*game), getFileName(component));
                if (componentPath.isEmpty()) {
                    continue;  // Skip components that don't exist
                }

                // Verify that the path actually exists
                QFileInfo fileInfo(componentPath);
                if (!fileInfo.exists() || !fileInfo.isFile()) {
                    pathInvalid = true;
                }

                iconPath = getIconPath(component);
                parameters = QString();  // No parameters for default games
                extendedRedir = component.extendedRedir;
                shuntCount = component.shuntCount;
                noSpawning = (component.flags & MPQD_NO_SPAWNING) != 0;
            }

            QListWidgetItem *item = new QListWidgetItem(gameList);
            item->setText(displayText);

            // Set icon
            QIcon icon(iconPath.isEmpty() ? getIconPath(component) : iconPath);
            item->setIcon(icon);

            // Store data in item
            item->setData(Qt::UserRole, extendedRedir);
            item->setData(Qt::UserRole + 1, componentPath);
            item->setData(Qt::UserRole + 2, shuntCount);
            item->setData(Qt::UserRole + 3, noSpawning);
            item->setData(Qt::UserRole + 4, isModified);  // Track if this is from QSettings
            item->setData(Qt::UserRole + 5, parameters);  // Command-line parameters

            // Color coding and tooltips
            if (pathInvalid) {
                item->setForeground(QBrush(QColor(255, 0, 0)));  // Red
                QFont font = item->font();
                font.setItalic(true);
                item->setFont(font);
                if (isModified) {
                    item->setToolTip("Executable not found - Right click to reset");
                } else {
                    item->setToolTip("Executable not found");
                }
            } else if (isModified) {
                item->setForeground(QBrush(QColor(0, 0, 255)));  // Blue
                QFont font = item->font();
                font.setItalic(true);
                item->setFont(font);
                item->setToolTip("Default values modified - Right click to reset");
            }
        }
    }

    // Add novel custom applications (not in gamedata.cpp)
    // First, get all supported games to check if an app is in gamedata.cpp
    std::vector<SupportedGame> allGames = getSupportedGames();

    for (const QString& appName : customAppNames) {
        if (processedCustomApps.contains(appName)) {
            continue;  // Already processed as an override
        }

        settings.beginGroup("CustomApplications");
        settings.beginGroup(appName);
        QString componentPath = settings.value("executablePath").toString();
        QString iconPath = settings.value("iconPath").toString();
        QString parameters = settings.value("parameters").toString();
        bool extendedRedir = settings.value("extendedRedir", true).toBool();
        int shuntCount = settings.value("shuntCount", 0).toInt();
        bool noSpawning = settings.value("noSpawning", false).toBool();
        settings.endGroup();
        settings.endGroup();

        // Check if this app is in gamedata.cpp (but not detected on system)
        bool isInGameData = false;
        QString defaultIconPath;
        for (const SupportedGame& game : allGames) {
            for (const GameComponent& component : game.components) {
                QString displayText;
                if (game.components.size() == 1) {
                    displayText = QString::fromStdString(game.gameName);
                } else {
                    displayText = QString::fromStdString(game.gameName) + " - " + QString::fromStdString(component.componentName);
                }

                if (displayText == appName) {
                    isInGameData = true;
                    defaultIconPath = QString::fromStdString(component.iconPath);
                    break;
                }
            }
            if (isInGameData) break;
        }

        QListWidgetItem *item = new QListWidgetItem(gameList);
        item->setText(appName);

        // Set icon
        if (!iconPath.isEmpty()) {
            item->setIcon(QIcon(iconPath));
        } else if (!defaultIconPath.isEmpty()) {
            item->setIcon(QIcon(defaultIconPath));
        } else {
            item->setIcon(QIcon(":/icons/not-found.png"));
        }

        // Store data
        item->setData(Qt::UserRole, extendedRedir);
        item->setData(Qt::UserRole + 1, componentPath);
        item->setData(Qt::UserRole + 2, shuntCount);
        item->setData(Qt::UserRole + 3, noSpawning);
        item->setData(Qt::UserRole + 4, true);  // Is from QSettings
        item->setData(Qt::UserRole + 5, parameters);  // Command-line parameters

        // Verify path exists
        QFileInfo fileInfo(componentPath);
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            QFont font = item->font();
            font.setItalic(true);
            item->setFont(font);
            item->setForeground(QBrush(QColor(255, 0, 0)));  // Red
            if (isInGameData) {
                item->setToolTip("Executable not found - Right click to reset");
            } else {
                item->setToolTip("Executable not found - Right click to remove");
            }
        } else {
            QFont font = item->font();
            font.setItalic(true);
            item->setFont(font);
            item->setForeground(QBrush(QColor(0, 0, 255)));  // Blue
            if (isInGameData) {
                item->setToolTip("Default values modified - Right click to reset");
            } else {
                item->setToolTip("Custom application - Right click to remove");
            }
        }
    }

    // Show message if no games at all
    if (gameList->count() == 0) {
        QListWidgetItem *item = new QListWidgetItem(gameList);
        item->setText("No supported games detected");
        item->setIcon(QIcon(":/icons/not-found.png"));
        item->setFlags(Qt::NoItemFlags);  // Make it non-selectable
        item->setForeground(QBrush(QColor(128, 128, 128)));  // Gray text
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

void TargetSelectionPage::onGameListContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = gameList->itemAt(pos);
    if (!item) {
        return;
    }

    // Only show context menu for items from QSettings (UserRole + 4 == true)
    bool isFromSettings = item->data(Qt::UserRole + 4).toBool();
    if (!isFromSettings) {
        return;
    }

    QString appName = item->text();

    // Check if this is a default game (exists in gamedata.cpp)
    std::vector<SupportedGame> allGames = getSupportedGames();
    bool isDefaultGame = false;

    for (const SupportedGame& game : allGames) {
        for (const GameComponent& component : game.components) {
            QString displayText;
            if (game.components.size() == 1) {
                displayText = QString::fromStdString(game.gameName);
            } else {
                displayText = QString::fromStdString(game.gameName) + " - " + QString::fromStdString(component.componentName);
            }

            if (displayText == appName) {
                isDefaultGame = true;
                break;
            }
        }
        if (isDefaultGame) break;
    }

    // Create context menu
    QMenu contextMenu(this);
    QAction *action;

    if (isDefaultGame) {
        action = contextMenu.addAction("Reset to default values");
    } else {
        action = contextMenu.addAction("Remove from list");
    }

    // Show menu and handle action
    QAction *selectedAction = contextMenu.exec(gameList->mapToGlobal(pos));
    if (selectedAction == action) {
        // Remove from QSettings
        QSettings settings;
        settings.beginGroup("CustomApplications");
        settings.remove(appName);
        settings.endGroup();

        // Refresh the game list
        populateInstalledGames();
    }
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
    addButton = new QPushButton("Add &MPQs...", this);
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

void MPQSelectionPage::initializePage()
{
    loadSettings();
    QWizardPage::initializePage();
}

void MPQSelectionPage::cleanupPage()
{
    QWizardPage::cleanupPage();
}

void MPQSelectionPage::saveSettings()
{
    QSettings settings;
    settings.beginGroup("PatchWizard/MPQ");

    // Save last MPQ directory
    settings.setValue("lastDirectory", lastMPQDirectory);

    // Clear old MPQ entries first
    settings.remove("mpqs");

    // Save MPQ list with checked state (manual index management)
    settings.beginGroup("mpqs");
    for (int i = 0; i < mpqListWidget->count(); ++i) {
        settings.beginGroup(QString::number(i));
        QListWidgetItem *item = mpqListWidget->item(i);
        settings.setValue("path", item->text());
        settings.setValue("checked", item->checkState() == Qt::Checked);
        settings.endGroup();
    }
    settings.endGroup();

    settings.endGroup();
}

void MPQSelectionPage::loadSettings()
{
    QSettings settings;
    settings.beginGroup("PatchWizard/MPQ");

    // Block signals while loading to avoid triggering saves
    mpqListWidget->blockSignals(true);

    // Restore last MPQ directory
    lastMPQDirectory = settings.value("lastDirectory", QDir::currentPath()).toString();

    // Restore MPQ list (manual index management)
    settings.beginGroup("mpqs");
    int i = 0;
    while (true) {
        settings.beginGroup(QString::number(i));
        if (!settings.contains("path")) {
            settings.endGroup();
            break;
        }

        QString path = settings.value("path").toString();
        bool checked = settings.value("checked", true).toBool();
        settings.endGroup();

        // Only add if file still exists
        if (QFileInfo::exists(path)) {
            addMPQFile(path, checked);
        }

        ++i;
    }
    settings.endGroup();

    // Unblock signals
    mpqListWidget->blockSignals(false);

    settings.endGroup();
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
    saveSettings();  // Save whenever items change (check state, order, etc.)
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
    // Use last directory if available
    QString startDir = lastMPQDirectory.isEmpty() ? QDir::currentPath() : lastMPQDirectory;

    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        "Select MPQ Files",
        startDir,
        "MPQ Archives (*.mpq);;All Files (*.*)"
    );

    // Save the directory for next time
    if (!fileNames.isEmpty()) {
        lastMPQDirectory = QFileInfo(fileNames.first()).absolutePath();
    }

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
    saveSettings();  // Save after adding MPQs
}

void MPQSelectionPage::onAddFolderClicked()
{
    // Use last directory if available
    QString startDir = lastMPQDirectory.isEmpty() ? QDir::currentPath() : lastMPQDirectory;

    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "Select Folder Containing MPQ Files",
        startDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (folderPath.isEmpty()) {
        return;  // User cancelled
    }

    // Save the directory for next time
    lastMPQDirectory = folderPath;

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
    saveSettings();  // Save after adding folder
}

void MPQSelectionPage::onRemoveClicked()
{
    QList<QListWidgetItem*> selected = mpqListWidget->selectedItems();
    for (QListWidgetItem *item : selected) {
        delete item;
    }
    saveSettings();  // Save after removing MPQs
    validateMPQList();
}

void MPQSelectionPage::onMoveUpClicked()
{
    int currentRow = mpqListWidget->currentRow();
    if (currentRow > 0) {
        QListWidgetItem *item = mpqListWidget->takeItem(currentRow);
        mpqListWidget->insertItem(currentRow - 1, item);
        mpqListWidget->setCurrentRow(currentRow - 1);
        saveSettings();  // Save after reordering
    }
}

void MPQSelectionPage::onMoveDownClicked()
{
    int currentRow = mpqListWidget->currentRow();
    if (currentRow >= 0 && currentRow < mpqListWidget->count() - 1) {
        QListWidgetItem *item = mpqListWidget->takeItem(currentRow);
        mpqListWidget->insertItem(currentRow + 1, item);
        mpqListWidget->setCurrentRow(currentRow + 1);
        saveSettings();  // Save after reordering
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

    // Print all user choices to console
    qDebug() << "=== Patch Configuration ==="; // TODO: Cleanup
    qDebug() << "Target:" << targetPath; // TODO: Cleanup
    qDebug() << "Parameters:" << (parameters.isEmpty() ? "(none)" : parameters); // TODO: Cleanup
    qDebug() << "Extended Redir:" << (extendedRedir ? "Yes" : "No"); // TODO: Cleanup
    qDebug() << "Shunt Count:" << shuntCount; // TODO: Cleanup
    qDebug() << "No Spawning:" << (noSpawning ? "Yes" : "No"); // TODO: Cleanup
    qDebug() << "MPQs:" << mpqs; // TODO: Cleanup
    qDebug() << "Plugins:" << plugins; // TODO: Cleanup
    qDebug() << "==========================="; // TODO: Cleanup

    // TODO: Call the actual patcher DLL
}
