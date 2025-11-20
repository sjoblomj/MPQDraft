/*
    SEMPQWizard - Wizard for creating Self-Executing MPQ files
    
    This wizard has 3 pages:
    1. SEMPQ settings (name, icon, target)
    2. Select target executable
    3. Select plugins
*/

#ifndef SEMPQWIZARD_H
#define SEMPQWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QTabWidget>
#include <QListWidget>
#include <QSpinBox>
#include <cstdint>

// Forward declarations
class PluginPage;
struct GameComponent;

//=============================================================================
// Page 0: Introduction
//=============================================================================
class SEMPQIntroPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit SEMPQIntroPage(QWidget *parent = nullptr);
};

//=============================================================================
// Page 1: SEMPQ Settings
//=============================================================================
class SEMPQSettingsPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit SEMPQSettingsPage(QWidget *parent = nullptr);

    QString getSEMPQName() const;
    QString getMPQPath() const;
    QString getIconPath() const;

private slots:
    void onBrowseMPQClicked();
    void onBrowseIconClicked();
    void onMPQPathChanged();
    void onIconPathChanged();

private:
    void validateMPQPath();
    void updateIconPreview();

    QLineEdit *sempqNameEdit;
    QLineEdit *mpqPathEdit;
    QLineEdit *iconPathEdit;
    QPushButton *browseMPQButton;
    QPushButton *browseIconButton;
    QLabel *mpqIconLabel;
    QLabel *iconPreviewLabel;
};

//=============================================================================
// Page 2: Select Target Program
//=============================================================================
class SEMPQTargetPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit SEMPQTargetPage(QWidget *parent = nullptr);

    // Returns true if using registry-based target (Mode 1)
    bool isRegistryBased() const;

    // Mode 1: Registry-based - Supported Games (returns nullptr if not Mode 1)
    const GameComponent* getSelectedComponent() const;

    // Mode 2: Registry-based - Custom Registry (returns empty if not Mode 2)
    QString getCustomRegistryKey() const;
    QString getCustomRegistryValue() const;
    QString getCustomRegistryExe() const;
    QString getCustomRegistryTargetFile() const;
    int getCustomRegistryShuntCount() const;
    bool getCustomRegistryIsFullPath() const;
    uint32_t getCustomRegistryFlags() const;

    // Mode 3: Custom path (returns empty if not Mode 3)
    QString getCustomTargetPath() const;
    int getCustomTargetShuntCount() const;
    bool getCustomTargetNoSpawning() const;

    // Common to all modes
    QString getParameters() const;
    bool getExtendedRedir() const;

    // Override to validate page completion
    bool isComplete() const override;

private slots:
    void onGameSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onBrowseClicked();
    void onCustomPathChanged();
    void onCustomRegistryChanged();
    void onTabChanged(int index);
    void onExtendedRedirChanged(int state);
    void onPasteRegKeyClicked();
    void onPasteRegValueClicked();
    void onPasteExeFileClicked();
    void onPasteTargetFileClicked();

private:
    void validateCustomPath();
    void updateExtendedRedirCheckbox();
    void updateCustomRegistryPlaceholders();
    void clearWhitespaceOnlyFields();
    const GameComponent* getReferenceComponent() const;

    QTabWidget *tabWidget;

    // Supported Games tab (Mode 1)
    QListWidget *gameList;
    const GameComponent *selectedComponent;

    // Custom Registry tab (Mode 2)
    QLineEdit *customRegKeyEdit;
    QLineEdit *customRegValueEdit;
    QLineEdit *customRegExeEdit;
    QLineEdit *customRegTargetFileEdit;
    QSpinBox *customRegShuntCountSpinBox;
    QCheckBox *customRegIsFullPathCheckbox;
    QCheckBox *customRegNoSpawningCheckbox;

    // Paste buttons for Custom Registry
    QPushButton *pasteRegKeyButton;
    QPushButton *pasteRegValueButton;
    QPushButton *pasteExeFileButton;
    QPushButton *pasteTargetFileButton;

    // Labels showing reference game values for checkboxes and spinbox
    QLabel *isFullPathRefLabel;
    QLabel *noSpawningRefLabel;
    QLabel *shuntCountRefLabel;
    QLabel *extendedRedirRefLabel;

    // Custom Target tab (Mode 3)
    QLineEdit *customPathEdit;
    QPushButton *browseButton;
    QLabel *warningLabel;
    QSpinBox *customTargetShuntCountSpinBox;
    QCheckBox *customTargetNoSpawningCheckbox;

    // Common controls
    QLineEdit *parametersEdit;
    QCheckBox *extendedRedirCheckbox;

    // Track whether to warn about extended redir changes
    bool warnOnExtendedRedirChange;
};

//=============================================================================
// Main SEMPQ Wizard
//=============================================================================
class SEMPQWizard : public QWizard
{
    Q_OBJECT

public:
    explicit SEMPQWizard(QWidget *parent = nullptr);
    
    void accept() override;

private:
    SEMPQIntroPage *introPage;
    SEMPQSettingsPage *settingsPage;
    SEMPQTargetPage *targetPage;
    PluginPage *pluginPage;

    void createSEMPQ();
};

#endif // SEMPQWIZARD_H

