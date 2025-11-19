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

private:
    void validateMPQPath();

    QLineEdit *sempqNameEdit;
    QLineEdit *mpqPathEdit;
    QLineEdit *iconPathEdit;
    QPushButton *browseMPQButton;
    QPushButton *browseIconButton;
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

    // Mode 1: Registry-based (returns nullptr if Mode 2)
    const GameComponent* getSelectedComponent() const;

    // Mode 2: Custom path (returns empty if Mode 1)
    QString getCustomTargetPath() const;

    // Common to both modes
    QString getParameters() const;
    bool getExtendedRedir() const;

    // Override to validate page completion
    bool isComplete() const override;

private slots:
    void onGameSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onBrowseClicked();
    void onCustomPathChanged();
    void onTabChanged(int index);
    void onExtendedRedirChanged(int state);

private:
    void validateCustomPath();
    void updateExtendedRedirCheckbox();

    QTabWidget *tabWidget;

    // Supported Games tab (Mode 1)
    QListWidget *gameList;
    const GameComponent *selectedComponent;

    // Custom Target tab (Mode 2)
    QLineEdit *customPathEdit;
    QPushButton *browseButton;
    QLabel *warningLabel;

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

