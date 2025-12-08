/*
    SEMPQWizard - Wizard for creating Self-Executing MPQ files

    This wizard has 4 pages:
    0. Introduction
    1. SEMPQ settings (name, output, MPQ, icon)
    2. Select target executable
    3. Select plugins
*/

#include "sempqwizard.h"
#include "pluginpage.h"
#include "helpers/gamedata_qt.h"
#include "../../sempq/SEMPQCreator.h"
#include "helpers/sempqparamsbuilder.h"
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
#include <QApplication>
#include <QTimer>
#include <QSettings>
#include <QRegularExpression>
#include <QDir>

// Stylesheet for invalid input fields
static const char* INVALID_FIELD_STYLE = "QLineEdit { border: 2px solid #ff6b6b; background-color: #ffe0e0; }";

//=============================================================================
// Helper functions
//=============================================================================
static const QVector<SupportedGame>& getStaticGamesVector()
{
    static QVector<SupportedGame> games = getSupportedGamesQt();
    return games;
}


//=============================================================================
// Page 0: Introduction
//=============================================================================
SEMPQIntroPage::SEMPQIntroPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Welcome to SEMPQ Creation Wizard");
    setSubTitle("Create Self-Executing MPQ (SEMPQ) files.");
    setPixmap(
            QWizard::LogoPixmap,
            QPixmap(":/icons/mpqdraft.svg").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation)
    );

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *introLabel = new QLabel(
        "<p>Follow these steps to create a Self-Executing MPQ (SEMPQ) file.</p>"

        "<p><b>What are MPQs?</b><br>"
        "MPQs are archives containing game data such as graphics, sounds and other resources. "
        "They were used extensively by Blizzard Entertainment, but also Sierra OnLine's "
        "Lords of Magic.</p>"

        "<p><b>What is a SEMPQ?</b><br>"
        "A SEMPQ is a standalone executable that contains an MPQ archive and optional plugins. "
        "When run, it automatically patches and launches a game with the embedded modifications. "
        "The user does not need to have MPQDraft installed to run a SEMPQ.</p>"

        "<p><b>Benefits of SEMPQ files:</b></p>"
        "<ul>"
        "<li>Easy distribution - share a single .exe file with others.</li>"
        "<li>No installation required - recipients just run the file.</li>"
        "<li>Automatic patching - the game is patched and launched in one step.</li>"
        "<li>Self-contained - includes all necessary MPQ data and plugins.</li>"
        "</ul>"

        "<p>Click <b>Next</b> to configure your SEMPQ file.</p>"
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
// Page 1: SEMPQ Settings
//=============================================================================
SEMPQSettingsPage::SEMPQSettingsPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("SEMPQ Settings");
    setSubTitle("Configure the self-executing MPQ file to create.");
    setPixmap(
            QWizard::LogoPixmap,
            QPixmap(":/icons/mpq.svg").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation)
    );

    QVBoxLayout *layout = new QVBoxLayout(this);

    // SEMPQ name (max 31 chars - stored in fixed 32-byte buffer in STUBDATA)
    QLabel *nameLabel = new QLabel("SEMPQ Name:", this);
    layout->addWidget(nameLabel);
    sempqNameEdit = new QLineEdit(this);
    sempqNameEdit->setPlaceholderText("e.g., My Mod");
    sempqNameEdit->setMaxLength(31);
    layout->addWidget(sempqNameEdit);

    layout->addSpacing(20);

    // Source MPQ with icon to the left of both label and input
    QHBoxLayout *mpqSectionLayout = new QHBoxLayout();

    // MPQ icon on the left
    mpqIconLabel = new QLabel(this);
    QPixmap mpqPixmap(":/icons/mpq.svg");
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
    browseMPQButton = new QPushButton("Open &MPQ...", this);
    connect(browseMPQButton, &QPushButton::clicked, this, &SEMPQSettingsPage::onBrowseMPQClicked);
    connect(mpqPathEdit, &QLineEdit::textChanged, this, &SEMPQSettingsPage::onMPQPathChanged);
    mpqInputLayout->addWidget(mpqPathEdit);
    mpqInputLayout->addWidget(browseMPQButton);
    mpqVerticalLayout->addLayout(mpqInputLayout);

    mpqSectionLayout->addLayout(mpqVerticalLayout);
    layout->addLayout(mpqSectionLayout);

    layout->addSpacing(20);

    // Output path with save icon to the left of both label and input
    QHBoxLayout *outputSectionLayout = new QHBoxLayout();

    // Save icon on the left
    outputIconLabel = new QLabel(this);
    QPixmap savePixmap(":/icons/save.svg");
    outputIconLabel->setPixmap(savePixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    outputIconLabel->setFixedSize(64, 64);
    outputIconLabel->setAlignment(Qt::AlignTop);
    outputSectionLayout->addWidget(outputIconLabel);

    // Vertical layout for label and input
    QVBoxLayout *outputVerticalLayout = new QVBoxLayout();
    QLabel *outputLabel = new QLabel("Output Path:", this);
    outputVerticalLayout->addWidget(outputLabel);

    QHBoxLayout *outputInputLayout = new QHBoxLayout();
    outputPathEdit = new QLineEdit(this);
    outputPathEdit->setPlaceholderText("Where to save the SEMPQ file");
    browseOutputButton = new QPushButton("&Save SEMPQ...", this);
    connect(browseOutputButton, &QPushButton::clicked, this, &SEMPQSettingsPage::onBrowseOutputClicked);
    connect(outputPathEdit, &QLineEdit::textChanged, this, &SEMPQSettingsPage::onOutputPathChanged);
    outputInputLayout->addWidget(outputPathEdit);
    outputInputLayout->addWidget(browseOutputButton);
    outputVerticalLayout->addLayout(outputInputLayout);

    outputSectionLayout->addLayout(outputVerticalLayout);
    layout->addLayout(outputSectionLayout);

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
    browseIconButton = new QPushButton("Browse for &Icon...", this);
    connect(browseIconButton, &QPushButton::clicked, this, &SEMPQSettingsPage::onBrowseIconClicked);
    connect(iconPathEdit, &QLineEdit::textChanged, this, &SEMPQSettingsPage::onIconPathChanged);
    iconInputLayout->addWidget(iconPathEdit);
    iconInputLayout->addWidget(browseIconButton);
    iconVerticalLayout->addLayout(iconInputLayout);

    iconSectionLayout->addLayout(iconVerticalLayout);
    layout->addLayout(iconSectionLayout);

    // Make all browse buttons the same width
    int maxWidth = qMax(browseMPQButton   ->sizeHint().width(),
                   qMax(browseOutputButton->sizeHint().width(),
                        browseIconButton  ->sizeHint().width()));
    browseMPQButton   ->setFixedWidth(maxWidth);
    browseOutputButton->setFixedWidth(maxWidth);
    browseIconButton  ->setFixedWidth(maxWidth);

    layout->addStretch();

    // Connect text changes to completeChanged signal for validation
    connect(sempqNameEdit,  &QLineEdit::textChanged, this, &SEMPQSettingsPage::completeChanged);
    connect(mpqPathEdit,    &QLineEdit::textChanged, this, &SEMPQSettingsPage::completeChanged);
    connect(outputPathEdit, &QLineEdit::textChanged, this, &SEMPQSettingsPage::completeChanged);

    // Connect field changes to save settings
    connect(sempqNameEdit,  &QLineEdit::textChanged, this, &SEMPQSettingsPage::saveSettings);
    connect(mpqPathEdit,    &QLineEdit::textChanged, this, &SEMPQSettingsPage::saveSettings);
    connect(iconPathEdit,   &QLineEdit::textChanged, this, &SEMPQSettingsPage::saveSettings);
    connect(outputPathEdit, &QLineEdit::textChanged, this, &SEMPQSettingsPage::saveSettings);

    SEMPQSettingsPage::onIconPathChanged();
}

bool SEMPQSettingsPage::isComplete() const
{
    // Page is complete if all required fields are non-empty AND valid
    return !sempqNameEdit ->text().trimmed().isEmpty() &&
           !mpqPathEdit   ->text().trimmed().isEmpty() &&
           !outputPathEdit->text().trimmed().isEmpty() &&
           isMPQPathValid() &&
           isOutputPathValid();
}

bool SEMPQSettingsPage::validatePage()
{
    // Check if the output file already exists and prompt for overwriting
    QString outputPath = outputPathEdit->text().trimmed();
    QFileInfo outputFileInfo(outputPath);

    if (outputFileInfo.exists()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("File Already Exists");
        msgBox.setText(QString("The file '%1' already exists.").arg(outputPath));
        msgBox.setInformativeText("Do you want to overwrite it?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        int result = msgBox.exec();

        if (result == QMessageBox::No) {
            // User chose not to overwrite - stay on this page
            return false;
        }
    }

    // Allow proceeding to next page
    return true;
}

void SEMPQSettingsPage::initializePage()
{
    loadSettings();
    QWizardPage::initializePage();
}

void SEMPQSettingsPage::cleanupPage()
{
    QWizardPage::cleanupPage();
}

void SEMPQSettingsPage::saveSettings()
{
    QSettings settings;
    settings.beginGroup("SEMPQWizard/Settings");

    settings.setValue("sempqName",   sempqNameEdit->text());
    settings.setValue("mpqPath",       mpqPathEdit->text());
    settings.setValue("iconPath",     iconPathEdit->text());
    settings.setValue("outputPath", outputPathEdit->text());

    settings.endGroup();
}

void SEMPQSettingsPage::loadSettings()
{
    QSettings settings;
    settings.beginGroup("SEMPQWizard/Settings");

    // Block signals while loading to avoid triggering saves
    sempqNameEdit ->blockSignals(true);
    mpqPathEdit   ->blockSignals(true);
    iconPathEdit  ->blockSignals(true);
    outputPathEdit->blockSignals(true);

    sempqNameEdit ->setText(settings.value("sempqName",  "").toString());
    mpqPathEdit   ->setText(settings.value("mpqPath",    "").toString());
    iconPathEdit  ->setText(settings.value("iconPath",   "").toString());
    outputPathEdit->setText(settings.value("outputPath", "").toString());

    // Unblock signals
    sempqNameEdit ->blockSignals(false);
    mpqPathEdit   ->blockSignals(false);
    iconPathEdit  ->blockSignals(false);
    outputPathEdit->blockSignals(false);

    settings.endGroup();

    // Trigger validation and icon preview update
    onMPQPathChanged();
    onIconPathChanged();
    onOutputPathChanged();
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

QString SEMPQSettingsPage::getOutputPath() const
{
    return outputPathEdit->text();
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
    QSettings settings;
    QString startDir = settings.value("SEMPQWizard/Directories/mpq", QString()).toString();

    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select MPQ File",
        startDir,
        "MPQ Archives (*.mpq);;All Files (*.*)"
    );

    if (!fileName.isEmpty()) {
        mpqPathEdit->setText(fileName);
        settings.setValue("SEMPQWizard/Directories/mpq", QFileInfo(fileName).absolutePath());
        outputPathEdit->setFocus();
    }
}

void SEMPQSettingsPage::onBrowseOutputClicked()
{
    QSettings settings;
    QString startDir = settings.value("SEMPQWizard/Directories/output", QString()).toString();

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save SEMPQ File As",
        startDir,
        "Executable Files (*.exe);;All Files (*.*)"
    );

    if (!fileName.isEmpty()) {
        outputPathEdit->setText(fileName);
        settings.setValue("SEMPQWizard/Directories/output", QFileInfo(fileName).absolutePath());
        iconPathEdit->setFocus();
    }
}

void SEMPQSettingsPage::onBrowseIconClicked()
{
    QSettings settings;
    QString startDir = settings.value("SEMPQWizard/Directories/icon", QString()).toString();

    QString fileName = QFileDialog::getOpenFileName(
            this,
            "Select Icon File",
            startDir,
            "Icon Files (*.ico);;All Files (*.*)"
    );

    if (!fileName.isEmpty()) {
        iconPathEdit->setText(fileName);
        settings.setValue("SEMPQWizard/Directories/icon", QFileInfo(fileName).absolutePath());
        iconPathEdit->selectAll();
        iconPathEdit->setFocus();
    }
}

void SEMPQSettingsPage::onIconPathChanged()
{
    updateIconPreview();
    validateIconPath();
}

void SEMPQSettingsPage::onOutputPathChanged()
{
    validateOutputPath();
}

void SEMPQSettingsPage::validateIconPath()
{
    QString iconPath = iconPathEdit->text().trimmed();

    if (iconPath.isEmpty()) {
        iconPathEdit->setStyleSheet("");
        iconPathEdit->setToolTip("");
        return;
    }

    QFileInfo fileInfo(iconPath);
    if (!fileInfo.exists()) {
        iconPathEdit->setStyleSheet(INVALID_FIELD_STYLE);
        iconPathEdit->setToolTip("File does not exist");
    } else if (!fileInfo.isFile()) {
        iconPathEdit->setStyleSheet(INVALID_FIELD_STYLE);
        iconPathEdit->setToolTip("Path is not a file");
    } else {
        // Check if the icon can be loaded
        QPixmap testPixmap(iconPath);
        if (testPixmap.isNull()) {
            iconPathEdit->setStyleSheet(INVALID_FIELD_STYLE);
            iconPathEdit->setToolTip("File is not a valid icon");
        } else {
            iconPathEdit->setStyleSheet("");
            iconPathEdit->setToolTip("");
        }
    }
}

bool SEMPQSettingsPage::isMPQPathValid() const
{
    QString mpqPath = mpqPathEdit->text().trimmed();

    if (mpqPath.isEmpty()) {
        return false;
    }

    QFileInfo fileInfo(mpqPath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }

    return true;
}

void SEMPQSettingsPage::validateOutputPath()
{
    QString outputPath = outputPathEdit->text().trimmed();

    if (outputPath.isEmpty()) {
        outputPathEdit->setStyleSheet("");
        outputPathEdit->setToolTip("");
        return;
    }

    QString errorMessage;
    if (!isOutputPathValid(&errorMessage)) {
        outputPathEdit->setStyleSheet(INVALID_FIELD_STYLE);
        outputPathEdit->setToolTip(errorMessage);
        return;
    }

    // If the file exists, show a warning style (yellow) but don't mark as invalid
    QFileInfo fileInfo(outputPath);
    if (fileInfo.exists()) {
        outputPathEdit->setStyleSheet("QLineEdit { border: 2px solid #ffa500; background-color: #fff4e0; }");
        outputPathEdit->setToolTip("Warning: File already exists and will be overwritten");
    } else {
        outputPathEdit->setStyleSheet("");
        outputPathEdit->setToolTip("");
    }
}

bool SEMPQSettingsPage::isOutputPathValid(QString *errorMessage) const
{
    QString outputPath = outputPathEdit->text().trimmed();

    if (outputPath.isEmpty()) {
        if (errorMessage) *errorMessage = "Output path is empty";
        return false;
    }

    // Check for illegal characters in the path
    // Windows: < > " | ? * and control characters (0-31)
    QRegularExpression illegalChars;
    illegalChars.setPattern("[<>\"|?*\\x00-\\x1F]");

    QRegularExpressionMatch match = illegalChars.match(outputPath);
    if (match.hasMatch()) {
        if (errorMessage) *errorMessage = QString("Path contains illegal character: '%1'").arg(match.captured(0));
        return false;
    }

    // Check if the directory exists (not the file itself, just the parent directory)
    QFileInfo fileInfo(outputPath);
    QDir parentDir = fileInfo.absoluteDir();

    if (!parentDir.exists()) {
        if (errorMessage) *errorMessage = "Directory does not exist";
        return false;
    }

    // Check if we have write permission to the directory
    QFileInfo dirInfo(parentDir.absolutePath());
    if (!dirInfo.isWritable()) {
        if (errorMessage) *errorMessage = "No write permission to directory";
        return false;
    }

    // File existence is just a warning, not an error
    return true;
}

void SEMPQSettingsPage::updateIconPreview()
{
    QString iconPath = iconPathEdit->text().trimmed();

    // Try to load custom icon, fall back to default StarDraft icon
    QPixmap pixmap(iconPath);
    if (pixmap.isNull()) {
        pixmap.load(":/icons/StarDraft.png");
    }
    iconPreviewLabel->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}


//=============================================================================
// Page 2: Select Target Program
//=============================================================================
SEMPQTargetPage::SEMPQTargetPage(QWidget *parent)
    : QWizardPage(parent), selectedComponent(nullptr), warnOnExtendedRedirChange(true)
{
    setTitle("Select Target Program");
    setSubTitle("Choose the program that the SEMPQ will launch.");
    setPixmap(
            QWizard::LogoPixmap,
            QPixmap(":/icons/blizzard/bnet.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation)
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    // Create a tab widget (don't connect signal yet - will do after all widgets are created)
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
        "it portable across different machines.",
        supportedGamesTab);
    gamesInfoLabel->setWordWrap(true);
    gamesLayout->addWidget(gamesInfoLabel);

    gamesLayout->addSpacing(10);

    // Game list
    gameList = new QListWidget(supportedGamesTab);
    gameList->setIconSize(QSize(32, 32));
    connect(gameList, &QListWidget::currentItemChanged, this, &SEMPQTargetPage::onGameSelectionChanged);

    // Populate the game list with all components from all games.
    // Use shared static storage so pointers remain valid
    const QVector<SupportedGame>& games = getStaticGamesVector();
    for (const SupportedGame& game : games) {
        for (const GameComponent& component : game.components) {
            QListWidgetItem *item = new QListWidgetItem(gameList);

            QString displayText;
            if (game.components.size() == 1) {
                // Single component - just show the game name
                displayText = getGameName(game);
            } else {
                // Multiple components - show "Game - Component"
                displayText = QString("%1 - %2").arg(getGameName(game), getComponentName(component));
            }
            item->setText(displayText);

            // Set icon
            QIcon icon(getIconPath(component));
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

    // Set background to match the tab background (white)
    QPalette scrollPalette = customRegScrollArea->palette();
    scrollPalette.setColor(QPalette::Window, Qt::white);
    customRegScrollArea->setPalette(scrollPalette);
    customRegScrollArea->setAutoFillBackground(true);

    // Create the content widget that will go inside the scroll area
    QWidget *customRegContentWidget = new QWidget();
    QVBoxLayout *customRegLayout = new QVBoxLayout(customRegContentWidget);

    QLabel *customRegInfoLabel = new QLabel(
        "If the game you want to target is not in the list of Supported Games, "
        "you can specify here the Windows registry data for how to find it, "
        "along with other settings. If this is correctly configured, the SEMPQ "
        "will work on any computer where the game is properly installed.<br><br>"
        "By selecting a game in the Supported Games tab, you will see below its "
        "value for each field. Use the Paste button next to the field to fill "
        "in the value from the selected game.",
        customRegContentWidget);
    customRegInfoLabel->setWordWrap(true);
    customRegLayout->addWidget(customRegInfoLabel);

    customRegLayout->addSpacing(10);

    // Registry Key
    QHBoxLayout *regKeyLabelLayout = new QHBoxLayout();
    QLabel *regKeyLabel = new QLabel("Registry Key:", customRegContentWidget);
    regKeyLabelLayout->addWidget(regKeyLabel);

    QLabel *regKeyHelp = new QLabel(customRegContentWidget);
    regKeyHelp->setText(" ? ");
    regKeyHelp->setStyleSheet(
        "QLabel { background-color: #0079ff; color: white; border-radius: 10px; "
        "font-weight: bold; font-size: 12px; padding: 2px; min-width: 16px; "
        "max-width: 16px; min-height: 16px; max-height: 16px; "
        "qproperty-alignment: AlignCenter; }");
    regKeyHelp->setToolTip(
        "<b>Registry Key</b><br><br>"
        "The Windows registry path where the game stores its installation information.<br><br>"
        "<b>Example:</b> SOFTWARE\\Blizzard Entertainment\\StarCraft<br><br>"
        "To find this, open Registry Editor (regedit.exe) and navigate to HKEY_LOCAL_MACHINE "
        "or HKEY_CURRENT_USER to find where your game stores its installation path.");
    regKeyHelp->setCursor(Qt::WhatsThisCursor);
    regKeyLabelLayout->addWidget(regKeyHelp);
    regKeyLabelLayout->addStretch();
    customRegLayout->addLayout(regKeyLabelLayout);

    QHBoxLayout *regKeyInputLayout = new QHBoxLayout();
    customRegKeyEdit = new QLineEdit(customRegContentWidget);
    connect(customRegKeyEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::onCustomRegistryChanged);
    regKeyInputLayout->addWidget(customRegKeyEdit);

    pasteRegKeyButton = new QPushButton(customRegContentWidget);
    pasteRegKeyButton->setIcon(QIcon(":/icons/paste.svg"));
    pasteRegKeyButton->setMaximumWidth(30);
    pasteRegKeyButton->setToolTip("Copy value from selected game");
    connect(pasteRegKeyButton, &QPushButton::clicked, this, &SEMPQTargetPage::onPasteRegKeyClicked);
    regKeyInputLayout->addWidget(pasteRegKeyButton);

    customRegLayout->addLayout(regKeyInputLayout);

    customRegLayout->addSpacing(5);

    // Registry Value
    QHBoxLayout *regValueLabelLayout = new QHBoxLayout();
    QLabel *regValueLabel = new QLabel("Registry Value Name:", customRegContentWidget);
    regValueLabelLayout->addWidget(regValueLabel);

    QLabel *regValueHelp = new QLabel(customRegContentWidget);
    regValueHelp->setText(" ? ");
    regValueHelp->setStyleSheet(
        "QLabel { background-color: #0079ff; color: white; border-radius: 10px; "
        "font-weight: bold; font-size: 12px; padding: 2px; min-width: 16px; "
        "max-width: 16px; min-height: 16px; max-height: 16px; "
        "qproperty-alignment: AlignCenter; }");
    regValueHelp->setToolTip(
        "<b>Registry Value Name</b><br><br>"
        "The name of the registry value that contains the game's installation path.<br><br>"
        "<b>Common examples:</b>"
        "<ul>"
        "<li>InstallPath</li>"
        "<li>Directory</li>"
        "<li>DiabloInstall</li></ul>"
        "Look in the registry key you specified above to find the exact value name.");
    regValueHelp->setCursor(Qt::WhatsThisCursor);
    regValueLabelLayout->addWidget(regValueHelp);
    regValueLabelLayout->addStretch();
    customRegLayout->addLayout(regValueLabelLayout);

    QHBoxLayout *regValueInputLayout = new QHBoxLayout();
    customRegValueEdit = new QLineEdit(customRegContentWidget);
    connect(customRegValueEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::onCustomRegistryChanged);
    regValueInputLayout->addWidget(customRegValueEdit);

    pasteRegValueButton = new QPushButton(customRegContentWidget);
    pasteRegValueButton->setIcon(QIcon(":/icons/paste.svg"));
    pasteRegValueButton->setMaximumWidth(30);
    pasteRegValueButton->setToolTip("Copy value from selected game");
    connect(pasteRegValueButton, &QPushButton::clicked, this, &SEMPQTargetPage::onPasteRegValueClicked);
    regValueInputLayout->addWidget(pasteRegValueButton);

    customRegLayout->addLayout(regValueInputLayout);

    customRegLayout->addSpacing(5);

    // Executable file name
    QHBoxLayout *exeFileLabelLayout = new QHBoxLayout();
    QLabel *exeFileLabel = new QLabel("Executable file name:", customRegContentWidget);
    exeFileLabelLayout->addWidget(exeFileLabel);

    QLabel *exeFileHelp = new QLabel(customRegContentWidget);
    exeFileHelp->setText(" ? ");
    exeFileHelp->setStyleSheet(
        "QLabel { background-color: #0079ff; color: white; border-radius: 10px; "
        "font-weight: bold; font-size: 12px; padding: 2px; min-width: 16px; "
        "max-width: 16px; min-height: 16px; max-height: 16px; "
        "qproperty-alignment: AlignCenter; }");
    exeFileHelp->setToolTip(
        "<b>Executable file name</b><br><br>"
        "The name of the game's executable file that will be launched.<br><br>"
        "<b>Example:</b> StarCraft.exe<br><br>"
        "This file will be combined with the path from the registry to create the full "
        "path to the game executable. Make sure to include the .exe extension.<br><br>"
        "<b>Note:</b> This field is unused when 'Registry value contains full path to "
        "executable' is checked.");
    exeFileHelp->setCursor(Qt::WhatsThisCursor);
    exeFileLabelLayout->addWidget(exeFileHelp);
    exeFileLabelLayout->addStretch();
    customRegLayout->addLayout(exeFileLabelLayout);

    QHBoxLayout *exeFileInputLayout = new QHBoxLayout();
    customRegExeEdit = new QLineEdit(customRegContentWidget);
    connect(customRegExeEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::onCustomRegistryChanged);
    exeFileInputLayout->addWidget(customRegExeEdit);

    pasteExeFileButton = new QPushButton(customRegContentWidget);
    pasteExeFileButton->setIcon(QIcon(":/icons/paste.svg"));
    pasteExeFileButton->setMaximumWidth(30);
    pasteExeFileButton->setToolTip("Copy value from selected game");
    connect(pasteExeFileButton, &QPushButton::clicked, this, &SEMPQTargetPage::onPasteExeFileClicked);
    exeFileInputLayout->addWidget(pasteExeFileButton);

    customRegLayout->addLayout(exeFileInputLayout);
    customRegLayout->addSpacing(5);

    // Target file name
    QHBoxLayout *targetFileLabelLayout = new QHBoxLayout();
    QLabel *targetFileLabel = new QLabel("Target file name:", customRegContentWidget);
    targetFileLabelLayout->addWidget(targetFileLabel);

    QLabel *targetFileHelp = new QLabel(customRegContentWidget);
    targetFileHelp->setText(" ? ");
    targetFileHelp->setStyleSheet(
        "QLabel { background-color: #0079ff; color: white; border-radius: 10px; "
        "font-weight: bold; font-size: 12px; padding: 2px; min-width: 16px; "
        "max-width: 16px; min-height: 16px; max-height: 16px; "
        "qproperty-alignment: AlignCenter; }");
    targetFileHelp->setToolTip(
        "<b>Target file name</b><br><br>"
        "The file that MPQDraft will inject into and patch.<br><br>"
        "<b>Most common case:</b> This is the same as the Executable file name. "
        "For example, if the executable is 'StarCraft.exe', the target is also 'StarCraft.exe'.<br><br>"
        "<b>Special case:</b> Some games use a launcher that starts a different executable. "
        "For example, Diablo II has 'Diablo II.exe' (launcher) and 'Game.exe' (actual game). "
        "In such cases, the Executable Filename is 'Diablo II.exe' and the Target File Name is 'Game.exe'.<br><br>"
        "<b>Note:</b> This field is unused when 'Registry value contains full path to executable' is checked.");
    targetFileHelp->setCursor(Qt::WhatsThisCursor);
    targetFileLabelLayout->addWidget(targetFileHelp);
    targetFileLabelLayout->addStretch();
    customRegLayout->addLayout(targetFileLabelLayout);

    QHBoxLayout *targetFileInputLayout = new QHBoxLayout();
    customRegTargetFileEdit = new QLineEdit(customRegContentWidget);
    connect(customRegTargetFileEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::onCustomRegistryChanged);
    targetFileInputLayout->addWidget(customRegTargetFileEdit);

    pasteTargetFileButton = new QPushButton(customRegContentWidget);
    pasteTargetFileButton->setIcon(QIcon(":/icons/paste.svg"));
    pasteTargetFileButton->setMaximumWidth(30);
    pasteTargetFileButton->setToolTip("Copy value from selected game");
    connect(pasteTargetFileButton, &QPushButton::clicked, this, &SEMPQTargetPage::onPasteTargetFileClicked);
    targetFileInputLayout->addWidget(pasteTargetFileButton);

    customRegLayout->addLayout(targetFileInputLayout);

    customRegLayout->addSpacing(10);

    // Checkbox for "Registry value contains full path to executable"
    QHBoxLayout *isFullPathLayout = new QHBoxLayout();
    customRegIsFullPathCheckbox = new QCheckBox(
        "Registry value contains full path to executable",
        customRegContentWidget);
    connect(customRegIsFullPathCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onIsFullPathChanged);
    isFullPathLayout->addWidget(customRegIsFullPathCheckbox);

    QLabel *isFullPathHelp = new QLabel(customRegContentWidget);
    isFullPathHelp->setText(" ? ");
    isFullPathHelp->setStyleSheet(
        "QLabel { background-color: #0079ff; color: white; border-radius: 10px; "
        "font-weight: bold; font-size: 12px; padding: 2px; min-width: 16px; "
        "max-width: 16px; min-height: 16px; max-height: 16px; "
        "qproperty-alignment: AlignCenter; }");
    isFullPathHelp->setToolTip(
        "<b>Registry Value Contains Full Path</b><br><br>"
        "Controls how the registry value is interpreted when the SEMPQ runs on the end user's machine.<br><br>"
        "<b>Unchecked (default):</b> The registry value contains only the installation directory. "
        "The Executable File Name and Target File Name will be appended to this directory path.<br><br>"
        "<b>Example:</b> If the registry contains 'C:\\Program Files\\StarCraft' and the Executable File Name "
        "is 'StarCraft.exe', the final path will be 'C:\\Program Files\\StarCraft\\StarCraft.exe'.<br><br>"
        "<b>Checked:</b> The registry value already contains the complete path to the executable file. "
        "The Executable File Name and Target File Name fields are ignored.<br><br>"
        "<b>Example:</b> If the registry contains 'C:\\Program Files\\StarCraft\\StarCraft.exe', "
        "this path will be used directly.<br><br>"
        "Most games store only the directory path, so leave this unchecked unless you've verified "
        "that the registry value includes the .exe filename.");
    isFullPathHelp->setCursor(Qt::WhatsThisCursor);
    isFullPathLayout->addWidget(isFullPathHelp);

    isFullPathRefLabel = new QLabel(customRegContentWidget);
    isFullPathRefLabel->setStyleSheet("QLabel { color: #808080; }");  // Gray text
    isFullPathLayout->addWidget(isFullPathRefLabel);

    isFullPathLayout->addStretch();
    customRegLayout->addLayout(isFullPathLayout);

    customRegLayout->addSpacing(10);

    // Shunt Count
    QHBoxLayout *shuntCountLabelLayout = new QHBoxLayout();
    QLabel *shuntCountLabel = new QLabel("Shunt Count:", customRegContentWidget);
    shuntCountLabelLayout->addWidget(shuntCountLabel);

    QLabel *shuntCountHelp = new QLabel(customRegContentWidget);
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
    customRegLayout->addLayout(shuntCountLabelLayout);

    QHBoxLayout *shuntCountInputLayout = new QHBoxLayout();
    customRegShuntCountSpinBox = new QSpinBox(customRegContentWidget);
    customRegShuntCountSpinBox->setMinimum(0);
    customRegShuntCountSpinBox->setMaximum(INT_MAX);  // Unlimited (max int value)
    customRegShuntCountSpinBox->setValue(0);
    shuntCountInputLayout->addWidget(customRegShuntCountSpinBox);

    shuntCountRefLabel = new QLabel(customRegContentWidget);
    shuntCountRefLabel->setStyleSheet("QLabel { color: #808080; }");  // Gray text
    shuntCountInputLayout->addWidget(shuntCountRefLabel);

    shuntCountInputLayout->addStretch();
    customRegLayout->addLayout(shuntCountInputLayout);

    customRegLayout->addSpacing(10);

    // Command-Line Parameters
    QHBoxLayout *customRegParamsLayout = new QHBoxLayout();
    QLabel *customRegParamsLabel = new QLabel(tr("Command-Line Parameters (optional):"), customRegContentWidget);
    customRegParamsLayout->addWidget(customRegParamsLabel);

    QLabel *customRegParamsHelp = new QLabel(customRegContentWidget);
    customRegParamsHelp->setText(" ? ");
    customRegParamsHelp->setStyleSheet(
            "QLabel { background-color: #0079ff; color: white; border-radius: 10px; "
            "font-weight: bold; font-size: 12px; padding: 2px; min-width: 16px; "
            "max-width: 16px; min-height: 16px; max-height: 16px; "
            "qproperty-alignment: AlignCenter; }");
    customRegParamsHelp->setToolTip(
            tr("<b>Command-Line Parameters</b><br><br>"
               "Optional arguments that will be passed to the target executable when it launches.<br><br>"
               "These parameters are passed directly to the game executable and have no effect on MPQDraft itself."));
    customRegParamsHelp->setCursor(Qt::WhatsThisCursor);
    customRegParamsLayout->addWidget(customRegParamsHelp);
    customRegParamsLayout->addStretch();
    customRegLayout->addLayout(customRegParamsLayout);

    customRegParametersEdit = new QLineEdit(customRegContentWidget);
    customRegLayout->addWidget(customRegParametersEdit);

    customRegLayout->addSpacing(10);

    // Advanced flags section
    QLabel *flagsLabel = new QLabel("<b>Advanced Flags:</b>", customRegContentWidget);
    customRegLayout->addWidget(flagsLabel);

    QHBoxLayout *noSpawningLayout = new QHBoxLayout();
    customRegNoSpawningCheckbox = new QCheckBox(
        "Do not inject into child processes",
        customRegContentWidget);
    noSpawningLayout->addWidget(customRegNoSpawningCheckbox);

    QLabel *noSpawningHelp = new QLabel(customRegContentWidget);
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

    noSpawningRefLabel = new QLabel(customRegContentWidget);
    noSpawningRefLabel->setStyleSheet("QLabel { color: #808080; }");  // Gray text
    noSpawningLayout->addWidget(noSpawningRefLabel);

    noSpawningLayout->addStretch();
    customRegLayout->addLayout(noSpawningLayout);

    // Extended file redirection checkbox
    QHBoxLayout *customRegExtendedRedirLayout = new QHBoxLayout();

    customRegExtendedRedirCheckbox = new QCheckBox(tr("Use extended file redirection"), customRegContentWidget);
    customRegExtendedRedirCheckbox->setChecked(true);  // Default to true
    customRegExtendedRedirLayout->addWidget(customRegExtendedRedirCheckbox);

    QLabel *customRegExtendedRedirHelp = new QLabel(customRegContentWidget);
    customRegExtendedRedirHelp->setText(" ? ");
    customRegExtendedRedirHelp->setStyleSheet(
            "QLabel { background-color: #0079ff; color: white; border-radius: 10px; "
            "font-weight: bold; font-size: 12px; padding: 2px; min-width: 16px; "
            "max-width: 16px; min-height: 16px; max-height: 16px; "
            "qproperty-alignment: AlignCenter; }");
    customRegExtendedRedirHelp->setToolTip(
            tr("<b>Extended File Redirection (MPQD_EXTENDED_REDIR)</b><br><br>"
               "Blizzard games use Storm.dll to access MPQ archives. Some Storm functions "
               "(like SFileOpenFileEx) can bypass the normal MPQ priority chain by accepting "
               "a specific archive handle.<br><br>"
               "When enabled, MPQDraft hooks these functions to force them to search through "
               "the entire MPQ priority chain (including your custom MPQs), even when the game "
               "tries to read from a specific archive.<br><br>"
               "<b>When to enable:</b> Most Blizzard games including StarCraft and Warcraft III "
               "require this for mods to work correctly.<br><br>"
               "<b>When to disable:</b> Only disable if you're certain the target program doesn't "
               "use these Storm functions, or if you experience compatibility issues."));
    customRegExtendedRedirHelp->setCursor(Qt::WhatsThisCursor);
    customRegExtendedRedirLayout->addWidget(customRegExtendedRedirHelp);

    extendedRedirRefLabel = new QLabel(customRegContentWidget);
    extendedRedirRefLabel->setStyleSheet("QLabel { color: #808080; }");  // Gray text
    customRegExtendedRedirLayout->addWidget(extendedRedirRefLabel);

    customRegExtendedRedirLayout->addStretch();
    customRegLayout->addLayout(customRegExtendedRedirLayout);

    customRegLayout->addStretch();

    // Set the content widget in the scroll area and add scroll area to tab
    customRegScrollArea->setWidget(customRegContentWidget);
    customRegTabLayout->addWidget(customRegScrollArea);

    //=========================================================================
    // Tab 3: Custom Target (Hardcoded Path)
    //=========================================================================
    QWidget *customTargetTab = new QWidget();
    QVBoxLayout *customTargetTabLayout = new QVBoxLayout(customTargetTab);
    customTargetTabLayout->setContentsMargins(0, 0, 0, 0);

    // Create a scroll area for custom target content
    QScrollArea *customTargetScrollArea = new QScrollArea(customTargetTab);
    customTargetScrollArea->setWidgetResizable(true);
    customTargetScrollArea->setFrameShape(QFrame::NoFrame);
    customTargetScrollArea->setStyleSheet("QScrollArea { background-color: white; }");

    // Create a content widget for the scroll area
    QWidget *customTargetContentWidget = new QWidget();
    customTargetContentWidget->setStyleSheet("QWidget { background-color: white; }");
    QVBoxLayout *customLayout = new QVBoxLayout(customTargetContentWidget);

    QLabel *customInfoLabel = new QLabel(
        "Specify a custom program path. This can be used for programs not in the "
        "list in the Supported Games tab.",
        customTargetContentWidget);
    customInfoLabel->setWordWrap(true);
    customLayout->addWidget(customInfoLabel);

    customLayout->addSpacing(10);

    // Warning box with icon
    QWidget *warningWidget = new QWidget(customTargetContentWidget);
    warningWidget->setStyleSheet(
        "QWidget { "
        "background-color: #fff3cd; "
        "border: 1px solid #ffc107; "
        "border-radius: 4px; "
        "}");
    QHBoxLayout *warningLayout = new QHBoxLayout(warningWidget);
    warningLayout->setContentsMargins(10, 10, 10, 10);
    warningLayout->setSpacing(8);

    // Warning icon
    QLabel *warningIcon = new QLabel(warningWidget);
    warningIcon->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(16, 16));
    warningIcon->setAlignment(Qt::AlignTop);
    warningIcon->setStyleSheet("QLabel { background-color: transparent; border: none; }");
    warningIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    warningLayout->addWidget(warningIcon);

    // Warning text
    warningLabel = new QLabel(
        "<b>Warning:</b> When using a custom path, the exact path you specify "
        "must exist on all computers where the SEMPQ will be run. For better "
        "portability, use the Supported Games tab instead.",
        warningWidget);
    warningLabel->setWordWrap(true);
    warningLabel->setStyleSheet("QLabel { color: #856404; background-color: transparent; border: none; }");
    warningLayout->addWidget(warningLabel, 1);  // Stretch factor of 1 to take remaining space

    customLayout->addWidget(warningWidget);

    customLayout->addSpacing(10);

    // Target path
    QLabel *targetLabel = new QLabel("Target Program Path:", customTargetContentWidget);
    customLayout->addWidget(targetLabel);

    QHBoxLayout *targetLayout = new QHBoxLayout();
    customPathEdit = new QLineEdit(customTargetContentWidget);
    connect(customPathEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::onCustomPathChanged);

    browseButton = new QPushButton("Bro&wse...", customTargetContentWidget);
    connect(browseButton, &QPushButton::clicked, this, &SEMPQTargetPage::onBrowseClicked);

    targetLayout->addWidget(customPathEdit);
    targetLayout->addWidget(browseButton);
    customLayout->addLayout(targetLayout);

    customLayout->addSpacing(10);

    // Shunt Count
    QHBoxLayout *customTargetShuntCountLabelLayout = new QHBoxLayout();
    QLabel *customTargetShuntCountLabel = new QLabel("Shunt Count:", customTargetContentWidget);
    customTargetShuntCountLabelLayout->addWidget(customTargetShuntCountLabel);

    QLabel *customTargetShuntCountHelp = new QLabel(customTargetContentWidget);
    customTargetShuntCountHelp->setText(" ? ");
    customTargetShuntCountHelp->setStyleSheet(
        "QLabel { background-color: #0079ff; color: white; border-radius: 10px; "
        "font-weight: bold; font-size: 12px; padding: 2px; min-width: 16px; "
        "max-width: 16px; min-height: 16px; max-height: 16px; "
        "qproperty-alignment: AlignCenter; }");
    customTargetShuntCountHelp->setToolTip(
        "<b>Shunt Count</b><br><br>"
        "The number of times the game restarts itself before MPQDraft activates patching.<br><br>"
        "<b>0 (default):</b> Activate immediately when the game starts. Use this for most games.<br><br>"
        "<b>1:</b> Wait for the game to restart itself once before activating. Some games with "
        "copy protection (like Diablo) restart themselves after checking the CD, so MPQDraft "
        "needs to wait for this restart.<br><br>"
        "<b>Higher values:</b> Rarely needed, but available if a game restarts multiple times "
        "during its startup sequence.");
    customTargetShuntCountHelp->setCursor(Qt::WhatsThisCursor);
    customTargetShuntCountLabelLayout->addWidget(customTargetShuntCountHelp);
    customTargetShuntCountLabelLayout->addStretch();
    customLayout->addLayout(customTargetShuntCountLabelLayout);

    customTargetShuntCountSpinBox = new QSpinBox(customTargetContentWidget);
    customTargetShuntCountSpinBox->setMinimum(0);
    customTargetShuntCountSpinBox->setMaximum(INT_MAX);
    customTargetShuntCountSpinBox->setValue(0);
    customLayout->addWidget(customTargetShuntCountSpinBox);

    customLayout->addSpacing(10);

    // Command-Line Parameters
    QHBoxLayout *customTargetParamsLayout = new QHBoxLayout();
    QLabel *customTargetParamsLabel = new QLabel(tr("Command-Line Parameters (optional):"), customTargetContentWidget);
    customTargetParamsLayout->addWidget(customTargetParamsLabel);

    QLabel *customTargetParamsHelp = new QLabel(customTargetContentWidget);
    customTargetParamsHelp->setText(" ? ");
    customTargetParamsHelp->setStyleSheet(
            "QLabel { background-color: #0079ff; color: white; border-radius: 10px; "
            "font-weight: bold; font-size: 12px; padding: 2px; min-width: 16px; "
            "max-width: 16px; min-height: 16px; max-height: 16px; "
            "qproperty-alignment: AlignCenter; }");
    customTargetParamsHelp->setToolTip(
            tr("<b>Command-Line Parameters</b><br><br>"
               "Optional arguments that will be passed to the target executable when it launches.<br><br>"
               "These parameters are passed directly to the game executable and have no effect on MPQDraft itself."));
    customTargetParamsHelp->setCursor(Qt::WhatsThisCursor);
    customTargetParamsLayout->addWidget(customTargetParamsHelp);
    customTargetParamsLayout->addStretch();
    customLayout->addLayout(customTargetParamsLayout);

    customTargetParametersEdit = new QLineEdit(customTargetContentWidget);
    customLayout->addWidget(customTargetParametersEdit);

    customLayout->addSpacing(10);

    // Advanced flags section
    QLabel *customTargetFlagsLabel = new QLabel("<b>Advanced Flags:</b>", customTargetContentWidget);
    customLayout->addWidget(customTargetFlagsLabel);

    QHBoxLayout *customTargetNoSpawningLayout = new QHBoxLayout();
    customTargetNoSpawningCheckbox = new QCheckBox(
        "Do not inject into child processes",
        customTargetContentWidget);
    customTargetNoSpawningLayout->addWidget(customTargetNoSpawningCheckbox);

    QLabel *customTargetNoSpawningHelp = new QLabel(customTargetContentWidget);
    customTargetNoSpawningHelp->setText(" ? ");
    customTargetNoSpawningHelp->setStyleSheet(
        "QLabel { background-color: #0079ff; color: white; border-radius: 10px; "
        "font-weight: bold; font-size: 12px; padding: 2px; min-width: 16px; "
        "max-width: 16px; min-height: 16px; max-height: 16px; "
        "qproperty-alignment: AlignCenter; }");
    customTargetNoSpawningHelp->setToolTip(
        "<b>Do Not Inject Into Child Processes (MPQD_NO_SPAWNING)</b><br><br>"
        "By default, MPQDraft injects itself into any child processes created by the game. "
        "This ensures that patches work even if the game launches additional executables.<br><br>"
        "<b>When to enable:</b> Some games launch helper processes (updaters, launchers, "
        "crash reporters) that don't need patching and may cause issues if MPQDraft injects "
        "into them. Enable this flag to prevent injection into child processes.<br><br>"
        "<b>When to disable (default):</b> Most games work fine with child process injection, "
        "and some games require it for patches to work correctly.");
    customTargetNoSpawningHelp->setCursor(Qt::WhatsThisCursor);
    customTargetNoSpawningLayout->addWidget(customTargetNoSpawningHelp);

    customTargetNoSpawningLayout->addStretch();
    customLayout->addLayout(customTargetNoSpawningLayout);

    // Extended file redirection checkbox
    QHBoxLayout *customTargetExtendedRedirLayout = new QHBoxLayout();

    customTargetExtendedRedirCheckbox = new QCheckBox(tr("Use extended file redirection"), customTargetContentWidget);
    customTargetExtendedRedirCheckbox->setChecked(true);  // Default to true
    customTargetExtendedRedirLayout->addWidget(customTargetExtendedRedirCheckbox);

    QLabel *customTargetExtendedRedirHelp = new QLabel(customTargetContentWidget);
    customTargetExtendedRedirHelp->setText(" ? ");
    customTargetExtendedRedirHelp->setStyleSheet(
            "QLabel { background-color: #0079ff; color: white; border-radius: 10px; "
            "font-weight: bold; font-size: 12px; padding: 2px; min-width: 16px; "
            "max-width: 16px; min-height: 16px; max-height: 16px; "
            "qproperty-alignment: AlignCenter; }");
    customTargetExtendedRedirHelp->setToolTip(
            tr("<b>Extended File Redirection (MPQD_EXTENDED_REDIR)</b><br><br>"
               "Blizzard games use Storm.dll to access MPQ archives. Some Storm functions "
               "(like SFileOpenFileEx) can bypass the normal MPQ priority chain by accepting "
               "a specific archive handle.<br><br>"
               "When enabled, MPQDraft hooks these functions to force them to search through "
               "the entire MPQ priority chain (including your custom MPQs), even when the game "
               "tries to read from a specific archive.<br><br>"
               "<b>When to enable:</b> Most Blizzard games including StarCraft and Warcraft III "
               "require this for mods to work correctly.<br><br>"
               "<b>When to disable:</b> Only disable if you're certain the target program doesn't "
               "use these Storm functions, or if you experience compatibility issues."));
    customTargetExtendedRedirHelp->setCursor(Qt::WhatsThisCursor);
    customTargetExtendedRedirLayout->addWidget(customTargetExtendedRedirHelp);

    customTargetExtendedRedirLayout->addStretch();
    customLayout->addLayout(customTargetExtendedRedirLayout);

    customLayout->addStretch();

    // Set the content widget in the scroll area and add scroll area to tab
    customTargetScrollArea->setWidget(customTargetContentWidget);
    customTargetTabLayout->addWidget(customTargetScrollArea);

    //=========================================================================
    // Add tabs to tab widget
    //=========================================================================
    tabWidget->addTab(supportedGamesTab, "Supported &Games");
    tabWidget->addTab(customRegistryTab, "Custom &Registry");
    tabWidget->addTab(customTargetTab,   "Custom &Target");

    mainLayout->addWidget(tabWidget);

    // Now that all widgets are created, connect the signals
    connect(tabWidget, &QTabWidget::currentChanged, this, &SEMPQTargetPage::onTabChanged);
    connect(customRegExtendedRedirCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onExtendedRedirChanged);
    connect(customTargetExtendedRedirCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onExtendedRedirChanged);

    // Connect field changes to save settings
    connect(gameList, &QListWidget::currentRowChanged, this, &SEMPQTargetPage::saveSettings);
    connect(tabWidget, &QTabWidget::currentChanged, this, &SEMPQTargetPage::saveSettings);
    connect(customRegKeyEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::saveSettings);
    connect(customRegValueEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::saveSettings);
    connect(customRegExeEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::saveSettings);
    connect(customRegTargetFileEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::saveSettings);
    connect(customRegShuntCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SEMPQTargetPage::saveSettings);
    connect(customRegIsFullPathCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::saveSettings);
    connect(customRegNoSpawningCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::saveSettings);
    connect(customRegParametersEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::saveSettings);
    connect(customRegExtendedRedirCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::saveSettings);
    connect(customPathEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::saveSettings);
    connect(customTargetShuntCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SEMPQTargetPage::saveSettings);
    connect(customTargetNoSpawningCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::saveSettings);
    connect(customTargetParametersEdit, &QLineEdit::textChanged, this, &SEMPQTargetPage::saveSettings);
    connect(customTargetExtendedRedirCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::saveSettings);

    // Initialize Custom Registry placeholders with the first game from gamedata
    updateCustomRegistryPlaceholders();
}

bool SEMPQTargetPage::isRegistryBased() const
{
    int index = tabWidget->currentIndex();
    return (index == 0 || index == 1);  // Tab 0: Supported Games, Tab 1: Custom Registry
}

SEMPQTargetSettings SEMPQTargetPage::getTargetSettings() const
{
    SEMPQTargetSettings settings;

    int tabIndex = tabWidget->currentIndex();
    if (tabIndex == 0) {
        // Mode 1: Supported Games (Registry-based)
        // For supported games, extendedRedir comes from the component's default
        settings.mode = SEMPQTargetMode::SUPPORTED_GAME;
        settings.selectedComponent = selectedComponent;
        settings.extendedRedir = selectedComponent ? selectedComponent->extendedRedir : true;

    } else if (tabIndex == 1) {
        // Mode 2: Custom Registry
        settings.mode = SEMPQTargetMode::CUSTOM_REGISTRY;
        settings.extendedRedir = customRegExtendedRedirCheckbox->isChecked();
        settings.customRegistryKey        = customRegKeyEdit       ->text().trimmed().toStdString();
        settings.customRegistryValue      = customRegValueEdit     ->text().trimmed().toStdString();
        settings.customRegistryExe        = customRegExeEdit       ->text().trimmed().toStdString();
        settings.customRegistryTargetFile = customRegTargetFileEdit->text().trimmed().toStdString();
        settings.customRegistryShuntCount = customRegShuntCountSpinBox ->value();
        settings.customRegistryIsFullPath = customRegIsFullPathCheckbox->isChecked();

        // Build flags for custom registry mode
        settings.customRegistryFlags = 0;
        if (customRegExtendedRedirCheckbox->isChecked()) {
            settings.customRegistryFlags |= MPQD_EXTENDED_REDIR;
        }
        if (customRegNoSpawningCheckbox->isChecked()) {
            settings.customRegistryFlags |= MPQD_NO_SPAWNING;
        }

    } else {
        // Mode 3: Custom Target (Hardcoded Path)
        settings.mode = SEMPQTargetMode::CUSTOM_PATH;
        settings.extendedRedir = customTargetExtendedRedirCheckbox->isChecked();
        settings.customTargetPath = customPathEdit->text().toStdString();
        settings.customTargetShuntCount = customTargetShuntCountSpinBox->value();
        settings.customTargetNoSpawning = customTargetNoSpawningCheckbox->isChecked();
    }

    return settings;
}

QString SEMPQTargetPage::getParameters() const
{
    int tabIndex = tabWidget->currentIndex();
    if (tabIndex == 1) {
        // Custom Registry tab
        return customRegParametersEdit->text();
    } else if (tabIndex == 2) {
        // Custom Target tab
        return customTargetParametersEdit->text();
    }
    // Supported Games tab - no parameters
    return QString();
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

    // Update the extended redir checkbox to show the default for this component
    updateExtendedRedirCheckbox();

    // Update Custom Registry placeholders to show values from the selected game
    updateCustomRegistryPlaceholders();

    // Emit completeChanged to update wizard buttons
    emit completeChanged();
}

void SEMPQTargetPage::onBrowseClicked()
{
    QSettings settings;
    QString startDir = settings.value("SEMPQWizard/Directories/customTarget", QString()).toString();

    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select Target Program",
        startDir,
        "Executable Files (*.exe);;All Files (*)"
    );

    if (!fileName.isEmpty()) {
        customPathEdit->setText(fileName);
        settings.setValue("SEMPQWizard/Directories/customTarget", QFileInfo(fileName).absolutePath());
        customTargetShuntCountSpinBox->setFocus();
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
    if (index == 1) { // If switching to the Custom Registry tab (index 1)
        clearWhitespaceOnlyFields();
        updateCustomRegistryPlaceholders();
    }

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
    // Temporarily disconnect to avoid triggering warning when we update programmatically
    disconnect(customRegExtendedRedirCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onExtendedRedirChanged);
    disconnect(customTargetExtendedRedirCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onExtendedRedirChanged);

    if (selectedComponent) {
        // Set Custom Registry checkbox to the default for the selected component
        customRegExtendedRedirCheckbox->setChecked(selectedComponent->extendedRedir);
    } else {
        // No component selected - default to true
        customRegExtendedRedirCheckbox->setChecked(true);
    }

    // Custom Target always defaults to true (most games need it)
    customTargetExtendedRedirCheckbox->setChecked(true);

    // Reset warning flag when changing games
    warnOnExtendedRedirChange = true;

    // Reconnect the signals
    connect(customRegExtendedRedirCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onExtendedRedirChanged);
    connect(customTargetExtendedRedirCheckbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onExtendedRedirChanged);
}

void SEMPQTargetPage::onExtendedRedirChanged(int state)
{
    Q_UNUSED(state);

    // Only warn if we haven't warned yet
    if (!warnOnExtendedRedirChange) {
        return;
    }

    // Determine which checkbox triggered this (based on current tab)
    int tabIndex = tabWidget->currentIndex();
    QCheckBox *checkbox = (tabIndex == 1) ? customRegExtendedRedirCheckbox : customTargetExtendedRedirCheckbox;

    // For Custom Registry tab, only warn if there's a selected component
    if (tabIndex == 1 && !selectedComponent) {
        return;
    }

    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(tr("Extended File Redirection"));
    msgBox.setText(tr("It is highly recommended that you do not change 'Use extended file redirection' "
                   "unless you are completely sure what you are doing. Do you wish to continue?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int result = msgBox.exec();

    if (result == QMessageBox::Yes) {
        // User confirmed - don't warn again
        warnOnExtendedRedirChange = false;
    } else {
        // User cancelled - revert the change
        disconnect(checkbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onExtendedRedirChanged);
        checkbox->setChecked(!checkbox->isChecked());
        connect(checkbox, &QCheckBox::stateChanged, this, &SEMPQTargetPage::onExtendedRedirChanged);
    }
}

void SEMPQTargetPage::onIsFullPathChanged(int state)
{
    Q_UNUSED(state);

    // Enable/disable the filename fields based on the checkbox state
    bool isFullPath = customRegIsFullPathCheckbox->isChecked();

    customRegExeEdit       ->setEnabled(!isFullPath);
    customRegTargetFileEdit->setEnabled(!isFullPath);
    pasteExeFileButton     ->setEnabled(!isFullPath);
    pasteTargetFileButton  ->setEnabled(!isFullPath);

    // Trigger validation update
    emit completeChanged();
}

bool SEMPQTargetPage::isComplete() const
{
    int index = tabWidget->currentIndex();

    if (index == 0) {
        // Tab 0: Supported Games - Must have a game selected
        return selectedComponent != nullptr;
    } else if (index == 1) {
        // Tab 1: Custom Registry - Must have all required fields filled
        bool hasKeyAndValue = !customRegKeyEdit  ->text().trimmed().isEmpty() &&
                              !customRegValueEdit->text().trimmed().isEmpty();

        // If "full path" is checked, filename fields are not required
        if (customRegIsFullPathCheckbox->isChecked()) {
            return hasKeyAndValue;
        } else {
            // If "full path" is unchecked, filename fields are required
            return hasKeyAndValue &&
                   !customRegExeEdit       ->text().trimmed().isEmpty() &&
                   !customRegTargetFileEdit->text().trimmed().isEmpty();
        }
    } else {
        // Tab 2: Custom Target - Must have a custom path entered
        return !customPathEdit->text().isEmpty();
    }
}

void SEMPQTargetPage::initializePage()
{
    loadSettings();
    QWizardPage::initializePage();
}

void SEMPQTargetPage::cleanupPage()
{
    QWizardPage::cleanupPage();
}

const GameComponent* SEMPQTargetPage::getReferenceComponent() const
{
    // If a game is selected in the Supported Games tab, use that
    if (selectedComponent) {
        return selectedComponent;
    }

    // Otherwise, use the first game from gamedata as default
    const QVector<SupportedGame>& games = getStaticGamesVector();
    if (!games.isEmpty() && !games[0].components.empty()) {
        return &games[0].components[0];
    }

    return nullptr;
}

void SEMPQTargetPage::clearWhitespaceOnlyFields()
{
    for (QLineEdit *field : {customRegKeyEdit, customRegValueEdit, customRegExeEdit, customRegTargetFileEdit}) {
        if (field->text().trimmed().isEmpty()) {
            field->clear();
        }
    }
}

void SEMPQTargetPage::saveSettings()
{
    QSettings settings;
    settings.beginGroup("SEMPQWizard/Target");

    // Save current tab
    settings.setValue("currentTab", tabWidget->currentIndex());

    // Save the selected game (if any)
    if (selectedComponent) {
        settings.setValue("selectedGame", gameList->currentItem()->text());
    }

    // Save Custom Registry settings
    settings.setValue("customRegKey",          customRegKeyEdit              ->text());
    settings.setValue("customRegValue",        customRegValueEdit            ->text());
    settings.setValue("customRegExe",          customRegExeEdit              ->text());
    settings.setValue("customRegTargetFile",   customRegTargetFileEdit       ->text());
    settings.setValue("customRegShuntCount",   customRegShuntCountSpinBox    ->value());
    settings.setValue("customRegIsFullPath",   customRegIsFullPathCheckbox   ->isChecked());
    settings.setValue("customRegNoSpawning",   customRegNoSpawningCheckbox   ->isChecked());
    settings.setValue("customRegParameters",   customRegParametersEdit       ->text());
    settings.setValue("customRegExtendedRedir",customRegExtendedRedirCheckbox->isChecked());

    // Save Custom Target settings
    settings.setValue("customTargetPath",         customPathEdit                   ->text());
    settings.setValue("customTargetShuntCount",   customTargetShuntCountSpinBox    ->value());
    settings.setValue("customTargetNoSpawning",   customTargetNoSpawningCheckbox   ->isChecked());
    settings.setValue("customTargetParameters",   customTargetParametersEdit       ->text());
    settings.setValue("customTargetExtendedRedir",customTargetExtendedRedirCheckbox->isChecked());

    settings.endGroup();
}

void SEMPQTargetPage::loadSettings()
{
    QSettings settings;
    settings.beginGroup("SEMPQWizard/Target");

    // Block signals while loading to avoid triggering saves
    const std::initializer_list<QObject*> widgets = {
        tabWidget, gameList,
        customRegKeyEdit, customRegValueEdit, customRegExeEdit, customRegTargetFileEdit,
        customRegShuntCountSpinBox, customRegIsFullPathCheckbox, customRegNoSpawningCheckbox,
        customRegParametersEdit, customRegExtendedRedirCheckbox,
        customPathEdit, customTargetShuntCountSpinBox, customTargetNoSpawningCheckbox,
        customTargetParametersEdit, customTargetExtendedRedirCheckbox
    };
    for (QObject *widget : widgets) {
        widget->blockSignals(true);
    }

    // Restore tab
    int savedTab = settings.value("currentTab", 0).toInt();
    tabWidget->setCurrentIndex(savedTab);

    // Save the extended redir values to restore later (after game selection updates them)
    bool savedCustomRegExtendedRedir = settings.value("customRegExtendedRedir", true).toBool();
    bool savedCustomTargetExtendedRedir = settings.value("customTargetExtendedRedir", true).toBool();

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

    // Restore Custom Registry settings
    customRegKeyEdit              ->setText   (settings.value("customRegKey",          "")   .toString());
    customRegValueEdit            ->setText   (settings.value("customRegValue",        "")   .toString());
    customRegExeEdit              ->setText   (settings.value("customRegExe",          "")   .toString());
    customRegTargetFileEdit       ->setText   (settings.value("customRegTargetFile",   "")   .toString());
    customRegShuntCountSpinBox    ->setValue  (settings.value("customRegShuntCount",   0)    .toInt());
    customRegIsFullPathCheckbox   ->setChecked(settings.value("customRegIsFullPath",   false).toBool());
    customRegNoSpawningCheckbox   ->setChecked(settings.value("customRegNoSpawning",   false).toBool());
    customRegParametersEdit       ->setText   (settings.value("customRegParameters",   "")   .toString());

    // Restore Custom Target settings
    customPathEdit                   ->setText   (settings.value("customTargetPath",         "")   .toString());
    customTargetShuntCountSpinBox    ->setValue  (settings.value("customTargetShuntCount",   0)    .toInt());
    customTargetNoSpawningCheckbox   ->setChecked(settings.value("customTargetNoSpawning",   false).toBool());
    customTargetParametersEdit       ->setText   (settings.value("customTargetParameters",   "")   .toString());

    // Unblock signals
    for (QObject *widget : widgets) {
        widget->blockSignals(false);
    }

    settings.endGroup();

    // Manually trigger updates that would have been triggered by signals.
    // This is necessary because we blocked signals during loading
    if (gameList->currentItem()) {
        onGameSelectionChanged(gameList->currentItem(), nullptr);
    }

    if (savedTab == 1) {        // Custom Registry tab
        onCustomRegistryChanged();
    } else if (savedTab == 2) { // Custom Target tab
        onCustomPathChanged();
    }
    onTabChanged(savedTab);

    // Restore the extended redir checkboxes AFTER all updates (which overwrite them with defaults).
    // Block signals to avoid triggering the warning dialog
    customRegExtendedRedirCheckbox->blockSignals(true);
    customRegExtendedRedirCheckbox->setChecked(savedCustomRegExtendedRedir);
    customRegExtendedRedirCheckbox->blockSignals(false);

    customTargetExtendedRedirCheckbox->blockSignals(true);
    customTargetExtendedRedirCheckbox->setChecked(savedCustomTargetExtendedRedir);
    customTargetExtendedRedirCheckbox->blockSignals(false);

    // Update the enabled state of filename fields based on isFullPath checkbox
    onIsFullPathChanged(0);
}

void SEMPQTargetPage::updateCustomRegistryPlaceholders()
{
    const GameComponent* refComp = getReferenceComponent();
    if (!refComp) {
        return;
    }

    // Find the game that contains this component to get registry info
    const QVector<SupportedGame>& games = getStaticGamesVector();
    const SupportedGame* refGame = nullptr;

    for (const SupportedGame& game : games) {
        for (const GameComponent& comp : game.components) {
            if (&comp == refComp) {
                refGame = &game;
                break;
            }
        }
        if (refGame) break;
    }

    if (!refGame) {
        return;
    }

    // Get the display name for the component
    QString displayName;
    if (refGame->components.size() == 1) {
        displayName = getGameName(*refGame);
    } else {
        displayName = getComponentName(*refComp);
    }

    // Update placeholders with reference game values
    customRegKeyEdit  ->setPlaceholderText(QString("%1: %2").arg(displayName, getRegistryKey(*refGame)));
    customRegValueEdit->setPlaceholderText(QString("%1: %2").arg(displayName, getRegistryValue(*refGame)));
    customRegExeEdit  ->setPlaceholderText(QString("%1: %2").arg(displayName, getFileName(*refComp)));

    // Target file name - always show the actual value
    customRegTargetFileEdit->setPlaceholderText(QString("%1: %2").arg(displayName, getTargetFileName(*refComp)));

    // Update paste button tooltips
    pasteRegKeyButton    ->setToolTip(QString("Copy value from %1").arg(displayName));
    pasteRegValueButton  ->setToolTip(QString("Copy value from %1").arg(displayName));
    pasteExeFileButton   ->setToolTip(QString("Copy value from %1").arg(displayName));
    pasteTargetFileButton->setToolTip(QString("Copy value from %1").arg(displayName));

    // Update reference labels for checkboxes and spinbox
    // For "value is full path" - this is always false for supported games (they store directory only)
    isFullPathRefLabel->setText(QString("%1: False").arg(displayName));

    // For "no spawning" - check if MPQD_NO_SPAWNING flag is set
    bool noSpawning = (refComp->flags & MPQD_NO_SPAWNING) != 0;
    noSpawningRefLabel->setText(QString("%1: %2").arg(displayName, noSpawning ? "True" : "False"));

    // For shunt count - show the value from the reference component
    shuntCountRefLabel->setText(QString("%1: %2").arg(displayName).arg(refComp->shuntCount));

    // For extended redir - show the value from the reference component
    extendedRedirRefLabel->setText(QString("%1: %2").arg(displayName, refComp->extendedRedir ? "True" : "False"));
}

void SEMPQTargetPage::onPasteRegKeyClicked()
{
    const GameComponent* refComp = getReferenceComponent();
    if (!refComp) return;

    // Find the game to get the registry key
    const QVector<SupportedGame>& games = getStaticGamesVector();
    for (const SupportedGame& game : games) {
        for (const GameComponent& comp : game.components) {
            if (&comp == refComp) {
                customRegKeyEdit->setText(getRegistryKey(game));
                return;
            }
        }
    }
}

void SEMPQTargetPage::onPasteRegValueClicked()
{
    const GameComponent* refComp = getReferenceComponent();
    if (!refComp) return;

    // Find the game to get registry value
    const QVector<SupportedGame>& games = getStaticGamesVector();
    for (const SupportedGame& game : games) {
        for (const GameComponent& comp : game.components) {
            if (&comp == refComp) {
                customRegValueEdit->setText(getRegistryValue(game));
                return;
            }
        }
    }
}

void SEMPQTargetPage::onPasteExeFileClicked()
{
    const GameComponent* refComp = getReferenceComponent();
    if (!refComp) return;

    customRegExeEdit->setText(getFileName(*refComp));
}

void SEMPQTargetPage::onPasteTargetFileClicked()
{
    const GameComponent* refComp = getReferenceComponent();
    if (!refComp) return;

    customRegTargetFileEdit->setText(getTargetFileName(*refComp));
}

//=============================================================================
// Page 4: Progress Page
//=============================================================================
SEMPQProgressPage::SEMPQProgressPage(QWidget *parent)
    : QWizardPage(parent)
    , creationComplete(false)
    , creationSuccess(false)
    , cancelRequested(false)
    , worker(nullptr)
    , currentPluginIndex(-1)
{
    setTitle("Creating SEMPQ");
    setSubTitle("Please wait while your SEMPQ file is being created...");

    // This is a final page - hide Back button, show only Cancel
    setFinalPage(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(20);

    // Status label - will show "Writing to file <path>"
    statusLabel = new QLabel("", this);
    statusLabel->setWordWrap(true);
    QFont statusFont = statusLabel->font();
    statusFont.setPointSize(10);
    statusLabel->setFont(statusFont);
    layout->addWidget(statusLabel);

    // Progress bar
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(false);
    layout->addWidget(progressBar);

    // Percentage label
    percentLabel = new QLabel("0%", this);
    percentLabel->setAlignment(Qt::AlignCenter);
    QFont percentFont = percentLabel->font();
    percentFont.setPointSize(12);
    percentFont.setBold(true);
    percentLabel->setFont(percentFont);
    layout->addWidget(percentLabel);

    // Progress log
    progressLog = new QTextEdit(this);
    progressLog->setReadOnly(true);
    progressLog->setMinimumHeight(200);
    progressLog->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    layout->addWidget(progressLog, 1);  // stretch factor 1 to expand
}

SEMPQProgressPage::~SEMPQProgressPage()
{
    // Make sure worker thread is stopped and cleaned up
    if (worker) {
        worker->requestCancellation();
        worker->wait();
        delete worker;
        worker = nullptr;
    }
}

void SEMPQProgressPage::cleanupPage()
{
    // Called when the user goes back or cancels the wizard.
    // Stop the worker thread if it's running
    if (worker) {
        worker->requestCancellation();
        worker->wait();
        delete worker;
        worker = nullptr;
    }

    QWizardPage::cleanupPage();
}

void SEMPQProgressPage::initializePage()
{
    // Reset state
    creationComplete = false;
    creationSuccess  = false;
    cancelRequested  = false;
    resultMessage.clear();

    progressBar ->setValue(0);
    percentLabel->setText("0%");
    statusLabel ->setText("Initializing...");
    progressLog ->clear();
    pluginNames.clear();
    currentPluginIndex = -1;

    // Get the list of plugins from the plugin page
    for (int i = 0; i < wizard()->pageIds().count(); i++) {
        int pageId = wizard()->pageIds().at(i);
        QWizardPage *page = wizard()->page(pageId);
        PluginPage *pluginPage = qobject_cast<PluginPage*>(page);

        if (pluginPage) {
            for (const std::string& plugin : pluginPage->getSelectedPluginPaths()) {
                // Extract just the filename from the full path
                QFileInfo fileInfo(QString::fromStdString(plugin));
                pluginNames.append(fileInfo.fileName());
            }
            break;
        }
    }

    // Initialize the progress log with all steps in the "not started" state
    rebuildProgressLog(0);

    // Hide the Back button by setting button layout to only show Cancel and Finish
    QList<QWizard::WizardButton> layout;
    layout << QWizard::Stretch << QWizard::CancelButton;
    wizard()->setButtonLayout(layout);

    // Set the icon based on user input and get the output path
    SEMPQWizard *sempqWizard = qobject_cast<SEMPQWizard*>(wizard());
    if (sempqWizard) {
        // Find the settings page to get the icon path and SEMPQ name
        SEMPQSettingsPage *settingsPage = nullptr;

        for (int i = 0; i < wizard()->pageIds().count(); i++) {
            int pageId = wizard()->pageIds().at(i);
            QWizardPage *page = wizard()->page(pageId);
            settingsPage = qobject_cast<SEMPQSettingsPage*>(page);
            if (settingsPage)
                break;
        }

        if (settingsPage) {
            QString iconPath = settingsPage->getIconPath();
            QPixmap iconPixmap;

            // Try to load the user's icon
            if (!iconPath.isEmpty() && QFileInfo::exists(iconPath)) {
                iconPixmap = QPixmap(iconPath);
            }

            // If the user icon failed or wasn't provided, use the StarDraft icon
            if (iconPixmap.isNull()) {
                iconPixmap = QPixmap(":/icons/StarDraft.png");
            }

            // Scale to 64x64 and set as the Wizard page logo
            if (!iconPixmap.isNull()) {
                setPixmap(QWizard::LogoPixmap, iconPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }

            // Set the status label to show the output path
            QString outputPath = settingsPage->getOutputPath();
            statusLabel->setText(QString("Writing to file: %1").arg(outputPath));
        }
    }

    // Start creation in the next event loop iteration
    QTimer::singleShot(100, this, &SEMPQProgressPage::startCreation);
}

bool SEMPQProgressPage::isComplete() const
{
    return creationComplete;
}

void SEMPQProgressPage::startCreation()
{
    // Clean up any existing worker
    if (worker) {
        worker->requestCancellation();
        worker->wait();
        delete worker;
        worker = nullptr;
    }

    // Create and start the worker thread
    worker = new SEMPQCreationWorker(this);
    worker->setWizard(wizard());

    // Connect signals
    connect(worker, &SEMPQCreationWorker::progressUpdate,   this, &SEMPQProgressPage::onProgressUpdate);
    connect(worker, &SEMPQCreationWorker::creationComplete, this, &SEMPQProgressPage::onCreationComplete);

    // Connect the wizard's Cancel button to the worker
    QPushButton *cancelButton = qobject_cast<QPushButton*>(wizard()->button(QWizard::CancelButton));
    if (cancelButton) {
        connect(cancelButton, &QPushButton::clicked, this, [this]() {
            if (worker) {
                worker->requestCancellation();
                cancelRequested = true;
                statusLabel->setText("Cancelling operation...");
            }
        });
    }

    worker->start();
}

void SEMPQProgressPage::rebuildProgressLog(int progress)
{
    QString html;
    QString doneIcon    = "[X]";    // Completed
    QString activeIcon  = "[>]";    // In progress
    QString pendingIcon = "[ ]";    // Not started

    // Step 1: Writing Executable Code
    if (progress >= SEMPQCreator::WRITE_PLUGINS_INITIAL_PROGRESS) { // Done
        html += QString("<span style='font-weight: bold; color: green;'>%1 Writing Executable Code</span><br>").arg(doneIcon);
    } else if (progress >= SEMPQCreator::WRITE_STUB_INITIAL_PROGRESS) { // In progress
        html += QString("<span style='font-style: italic; color: blue;'>%1 Writing Executable Code</span><br>").arg(activeIcon);
    } else { // Not started
        html += QString("<span style='color: gray;'>%1 Writing Executable Code</span><br>").arg(pendingIcon);
    }

    // Step 2: Writing Plugins
    int pluginCount = pluginNames.size();
    QString pluginStepLabel = QString("Writing %1 Plugin%2")
        .arg(pluginCount)
        .arg(pluginCount == 1 ? "" : "s");

    if (progress >= SEMPQCreator::WRITE_MPQ_INITIAL_PROGRESS) { // Done
        html += QString("<span style='font-weight: bold; color: green;'>%1 %2</span><br>").arg(doneIcon, pluginStepLabel);
    } else if (progress >= SEMPQCreator::WRITE_PLUGINS_INITIAL_PROGRESS) { // In progress
        html += QString("<span style='font-style: italic; color: blue;'>%1 %2</span><br>").arg(activeIcon, pluginStepLabel);
    } else { // Not started
        html += QString("<span style='color: gray;'>%1 %2</span><br>").arg(pendingIcon, pluginStepLabel);
    }

    // Individual plugins
    for (int i = 0; i < pluginNames.size(); i++) {
        QString pluginName = pluginNames[i];

        // Calculate progress for this plugin
        // Plugins span from WRITE_PLUGINS_INITIAL_PROGRESS to WRITE_MPQ_INITIAL_PROGRESS
        double pluginProgressRange = (double)SEMPQCreator::WRITE_PLUGINS_PROGRESS_SIZE / pluginNames.size();
        double pluginStartProgress = SEMPQCreator::WRITE_PLUGINS_INITIAL_PROGRESS + (i * pluginProgressRange);
        double pluginEndProgress   = pluginStartProgress + pluginProgressRange;

        if (progress >= pluginEndProgress) {
            html += QString("  <span style='font-weight: bold; color: green;'>%1 Writing plugin %2</span><br>")
                .arg(doneIcon, pluginName.toHtmlEscaped());
        } else if (progress >= pluginStartProgress && progress < pluginEndProgress) {
            html += QString("  <span style='font-style: italic; color: blue;'>%1 Writing plugin %2</span><br>")
                .arg(activeIcon, pluginName.toHtmlEscaped());
        } else {
            html += QString("  <span style='color: gray;'>%1 Writing plugin %2</span><br>")
                .arg(pendingIcon, pluginName.toHtmlEscaped());
        }
    }

    // Step 3: Writing MPQ Data
    if (progress >= SEMPQCreator::WRITE_FINISHED) { // Done
        html += QString("<span style='font-weight: bold; color: green;'>%1 Writing MPQ Data</span><br>").arg(doneIcon);
    } else if (progress >= SEMPQCreator::WRITE_MPQ_INITIAL_PROGRESS) { // In progress
        html += QString("<span style='font-style: italic; color: blue;'>%1 Writing MPQ Data</span><br>").arg(activeIcon);
    } else { // Not started
        html += QString("<span style='color: gray;'>%1 Writing MPQ Data</span><br>").arg(pendingIcon);
    }

    progressLog->setHtml(html);
}

void SEMPQProgressPage::updateProgressLog(const QString& text, int progress)
{
    // Just rebuild the entire log based on current progress
    rebuildProgressLog(progress);
}

void SEMPQProgressPage::onProgressUpdate(int progress, const QString& statusText)
{
    progressBar ->setValue(progress);
    percentLabel->setText(QString("%1%").arg(progress));

    // Update window title with percentage
    wizard()->setWindowTitle(QString("MPQDraft SEMPQ Wizard [%1%]").arg(progress));

    updateProgressLog(statusText, progress);
}

void SEMPQProgressPage::onCreationComplete(bool success, const QString& message)
{
    creationComplete = true;
    creationSuccess  = success;
    resultMessage    = message;

    // Restore window title (i.e. remove percentage)
    wizard()->setWindowTitle("MPQDraft SEMPQ Wizard");

    // Wait for worker thread to finish and clean up
    if (worker) {
        worker->wait();
        delete worker;
        worker = nullptr;
    }

    if (success) {
        percentLabel->setText("100%");
        progressBar ->setValue(SEMPQCreator::WRITE_FINISHED);

        // Add "Finished" to the progress log
        QString html = progressLog->toHtml();
        html += "<span style='font-weight: bold; color: green;'>[X] Finished - SEMPQ created successfully</span>";
        progressLog->setHtml(html);

        // Show the Finish button instead of Cancel
        QList<QWizard::WizardButton> layout;
        layout << QWizard::Stretch << QWizard::FinishButton;
        wizard()->setButtonLayout(layout);

        // Enable the Finish button
        wizard()->button(QWizard::FinishButton)->setEnabled(true);

        // Set focus on the Finish button
        wizard()->button(QWizard::FinishButton)->setFocus();

    } else {
        // Add "Failed" to the progress log
        QString html = progressLog->toHtml();
        html += "<span style='font-weight: bold; color: red;'>[X] Failed to create SEMPQ!</span>";
        progressLog->setHtml(html);

        // Show the error message
        QMessageBox::critical(this, "Error", message);

        // Show the Finish button to close
        QList<QWizard::WizardButton> layout;
        layout << QWizard::Stretch << QWizard::FinishButton;
        wizard()->setButtonLayout(layout);

        // Enable the Finish button
        wizard()->button(QWizard::FinishButton)->setEnabled(true);

        // Set focus on the Finish button
        wizard()->button(QWizard::FinishButton)->setFocus();
    }

    // Signal that the page is complete so the Finish button works
    emit completeChanged();
}

//=============================================================================
// SEMPQ Creation Worker Thread
//=============================================================================
SEMPQCreationWorker::SEMPQCreationWorker(QObject *parent)
    : QThread(parent)
    , wizard(nullptr)
    , creator(nullptr)
    , cancelRequested(false)
{
    creator = new SEMPQCreator();
}

SEMPQCreationWorker::~SEMPQCreationWorker()
{
    delete creator;
}

void SEMPQCreationWorker::setWizard(QWizard *w)
{
    wizard = w;
}

void SEMPQCreationWorker::requestCancellation()
{
    cancelRequested = true;
}

void SEMPQCreationWorker::run()
{
    if (!wizard) {
        emit creationComplete(false, "Internal error: wizard not set");
        return;
    }

    // Get wizard pages
    SEMPQWizard *sempqWizard = qobject_cast<SEMPQWizard*>(wizard);
    if (!sempqWizard) {
        emit creationComplete(false, "Internal error: invalid wizard type");
        return;
    }

    // Get the wizard pages - we need to access them through the wizard
    SEMPQSettingsPage *settingsPage = nullptr;
    SEMPQTargetPage *targetPage = nullptr;
    PluginPage *pluginPage = nullptr;

    // Find the pages by iterating through wizard pages
    for (int i = 0; i < wizard->pageIds().count(); i++) {
        int pageId = wizard->pageIds().at(i);
        QWizardPage *page = wizard->page(pageId);

        if (!settingsPage)
            settingsPage = qobject_cast<SEMPQSettingsPage*>(page);
        if (!targetPage)
            targetPage = qobject_cast<SEMPQTargetPage*>(page);
        if (!pluginPage)
            pluginPage = qobject_cast<PluginPage*>(page);
    }

    if (!settingsPage || !targetPage || !pluginPage) {
        emit creationComplete(false, "Internal error: could not find wizard pages");
        return;
    }

    // Extract basic settings from wizard pages
    SEMPQBasicSettings basicSettings;
    basicSettings.sempqName  = settingsPage->getSEMPQName() .toStdString();
    basicSettings.mpqPath    = settingsPage->getMPQPath()   .toStdString();
    basicSettings.iconPath   = settingsPage->getIconPath()  .toStdString();
    basicSettings.outputPath = settingsPage->getOutputPath().toStdString();
    basicSettings.parameters = targetPage  ->getParameters().toStdString();

    // Extract target settings from wizard pages
    SEMPQTargetSettings targetSettings = targetPage->getTargetSettings();

    // Get plugin modules with full metadata (component IDs, module IDs, etc.)
    std::vector<MPQDRAFTPLUGINMODULE> pluginModules = pluginPage->getSelectedPluginModules();

    // Build SEMPQ creation parameters
    std::string errorMessage;
    SEMPQCreationParams params;
    if (!SEMPQParamsBuilder::buildParams(basicSettings, targetSettings, pluginModules, params, errorMessage)) {
        emit creationComplete(false, QString::fromStdString(errorMessage));
        return;
    }

    executeCreation(params);
}

void SEMPQCreationWorker::executeCreation(const SEMPQCreationParams &params)
{
    // Create the progress callback (converts std::string to QString)
    auto progressCallback = [this](int progress, const std::string& statusText) {
        emit progressUpdate(progress, QString::fromStdString(statusText));
    };

    // Create the cancellation check
    auto cancellationCheck = [this]() -> bool {
        return cancelRequested;
    };

    std::string errorMessage;
    bool success = false;
#ifdef _WIN32
    success = creator->createSEMPQ(params, progressCallback, cancellationCheck, errorMessage);
#else
    errorMessage = "SEMPQ creation is not supported on this platform.";

    // Actual SEMPQ creation is not supported on non-Windows platforms.
    // One could here use the mock implementation for testing purposes,
    // in git commit 80169374858613ee275026206d1d22d071f051a9
    // The mock implementation has the same interface as the real:
    //success = creator->createSEMPQ(params, progressCallback, cancellationCheck, errorMessage);

    // Print all parameters to console
    qDebug() << "=== SEMPQ Creation Parameters ===";
    qDebug() << "SEMPQ Name:"   << QString::fromStdString(params.sempqName);
    qDebug() << "Output Path:"  << QString::fromStdString(params.outputPath);
    qDebug() << "MPQ Path:"     << QString::fromStdString(params.mpqPath);
    qDebug() << "Icon Path:"    << QString::fromStdString(params.iconPath);
    qDebug() << "Parameters:"   << QString::fromStdString(params.parameters);
    qDebug() << "Plugins:"      << static_cast<int>(params.pluginModules.size()) << "plugin(s)";
    for (size_t i = 0; i < params.pluginModules.size(); i++) {
        qDebug() << "  Plugin"  << (i + 1) << ":" << params.pluginModules[i].szModuleFileName;
    }
    qDebug() << "Use Registry:" << (params.useRegistry ? "Yes" : "No");
    if (params.useRegistry) {
        qDebug() << "Registry Key:"       << QString::fromStdString(params.registryKey);
        qDebug() << "Registry Value:"     << QString::fromStdString(params.registryValue);
        qDebug() << "Value Is Full Path:" << (params.valueIsFullPath ? "Yes" : "No");
        qDebug() << "Spawn File Name:"    << QString::fromStdString(params.spawnFileName);
        qDebug() << "Target File Name:"   << QString::fromStdString(params.targetFileName);
    } else {
        qDebug() << "Target Path:" << QString::fromStdString(params.targetPath);
    }
    qDebug() << "Shunt Count:"     << params.shuntCount;
    qDebug() << "Extended Redir:"  << (params.flags & MPQD_EXTENDED_REDIR ? "Yes" : "No");
    qDebug() << "No Spawning:"     << (params.flags & MPQD_NO_SPAWNING    ? "Yes" : "No");
    qDebug() << "=================================";
#endif

    if (success) {
        emit creationComplete(true, QString("SEMPQ file '%1' created successfully!").arg(QString::fromStdString(params.outputPath)));
    } else {
        emit creationComplete(false, QString::fromStdString(errorMessage));
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

    // Enable the minimize and maximize buttons - use the Window flag instead of Dialog
    setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    // Set window icon (needed after setWindowFlags resets it)
    setWindowIcon(QIcon(":/icons/mpqdraft.ico"));

    // Set the wizard sidebar image with margin and frame
    QPixmap originalPixmap(":/images/wizard.png");
    int innerMargin = 10;  // Space between frame and image
    int outerMargin = 25;  // Space between canvas edge and frame
    int frameWidth = 1;

    // Calculate total canvas size
    QPixmap pixmapWithMargin(originalPixmap.width()  + innerMargin * 2 + frameWidth * 2 + outerMargin * 2,
                             originalPixmap.height() + innerMargin * 2 + frameWidth * 2 + outerMargin * 2);
    pixmapWithMargin.fill(Qt::transparent);
    QPainter painter(&pixmapWithMargin);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Calculate positions
    int frameX = outerMargin;
    int frameY = outerMargin;
    int frameRectWidth  = originalPixmap.width()  + innerMargin * 2 + frameWidth * 2;
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
    introPage    = new SEMPQIntroPage(this);
    settingsPage = new SEMPQSettingsPage(this);
    targetPage   = new SEMPQTargetPage(this);
    pluginPage   = new PluginPage(this);
    progressPage = new SEMPQProgressPage(this);

    // Add pages
    addPage(introPage);
    addPage(settingsPage);
    addPage(targetPage);
    addPage(pluginPage);
    addPage(progressPage);

    // Set the minimum size
    setMinimumSize(900, 600);
}

void SEMPQWizard::accept()
{
    // The progress page will handle the actual creation.
    // Just call the base class accept() which will close the wizard.
    QWizard::accept();
}

void SEMPQWizard::reject()
{
    // Make sure the progress page cleans up its worker thread.
    // The cleanupPage() method will be called automatically.
    QWizard::reject();
}
