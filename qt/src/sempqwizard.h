/*
    SEMPQWizard - Wizard for creating Self-Executing MPQ files

    This wizard has 4 pages:
    0. Introduction
    1. SEMPQ settings (name, output, MPQ, icon)
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
#include <QProgressBar>
#include <QTextEdit>
#include <QThread>
#include <cstdint>

#include "core/sempqparamsbuilder.h"

// Forward declarations
class PluginPage;
class SEMPQCreator;
class SEMPQCreationWorker;

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
    QString getOutputPath() const;

    // Override to validate page completion
    bool isComplete() const override;
    bool validatePage() override;
    void initializePage() override;
    void cleanupPage() override;

private slots:
    void onBrowseMPQClicked();
    void onBrowseIconClicked();
    void onBrowseOutputClicked();
    void onMPQPathChanged();
    void onIconPathChanged();
    void onOutputPathChanged();

private:
    void validateMPQPath();
    void validateIconPath();
    void validateOutputPath();
    bool isMPQPathValid() const;
    bool isOutputPathValid(QString *errorMessage = nullptr) const;
    void updateIconPreview();
    void saveSettings();
    void loadSettings();

    QLineEdit *sempqNameEdit;
    QLineEdit *mpqPathEdit;
    QLineEdit *iconPathEdit;
    QLineEdit *outputPathEdit;
    QPushButton *browseMPQButton;
    QPushButton *browseIconButton;
    QPushButton *browseOutputButton;
    QLabel *mpqIconLabel;
    QLabel *iconPreviewLabel;
    QLabel *outputIconLabel;
};

//=============================================================================
// Page 2: Select Target Program
//=============================================================================
class SEMPQTargetPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit SEMPQTargetPage(QWidget *parent = nullptr);

    // Returns all target settings as a single struct
    SEMPQTargetSettings getTargetSettings() const;

    // Returns the command line parameters
    QString getParameters() const;

    // Override to validate page completion
    bool isComplete() const override;
    void initializePage() override;
    void cleanupPage() override;

private slots:
    void onGameSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onBrowseClicked();
    void onCustomPathChanged();
    void onCustomRegistryChanged();
    void onTabChanged(int index);
    void onExtendedRedirChanged(int state);
    void onIsFullPathChanged(int state);
    void onPasteRegKeyClicked();
    void onPasteRegValueClicked();
    void onPasteExeFileClicked();
    void onPasteTargetFileClicked();

private:
    bool isRegistryBased() const;
    void validateCustomPath();
    void updateExtendedRedirCheckbox();
    void updateCustomRegistryPlaceholders();
    void clearWhitespaceOnlyFields();
    const GameComponent* getReferenceComponent() const;
    void saveSettings();
    void loadSettings();

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
// Page 4: Progress Page
//=============================================================================
class SEMPQProgressPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit SEMPQProgressPage(QWidget *parent = nullptr);
    ~SEMPQProgressPage();

    void initializePage() override;
    bool isComplete() const override;
    void cleanupPage() override;

    void startCreation();

signals:
    void creationFinished(bool success, const QString& message);

private slots:
    void onProgressUpdate(int progress, const QString& statusText);
    void onCreationComplete(bool success, const QString& message);

private:
    QLabel *statusLabel;
    QProgressBar *progressBar;
    QLabel *percentLabel;
    QTextEdit *progressLog;

    bool creationComplete;
    bool creationSuccess;
    QString resultMessage;
    bool cancelRequested;

    SEMPQCreationWorker *worker;

    // Progress tracking
    QStringList pluginNames;
    int currentPluginIndex;

    void updateProgressLog(const QString& text, int progress);
    void rebuildProgressLog(int progress);
};

// Worker thread for SEMPQ creation
class SEMPQCreationWorker : public QThread
{
    Q_OBJECT

public:
    explicit SEMPQCreationWorker(QObject *parent = nullptr);
    ~SEMPQCreationWorker();

    void setWizard(QWizard *wizard);
    void requestCancellation();

signals:
    void progressUpdate(int progress, const QString& statusText);
    void creationComplete(bool success, const QString& message);

protected:
    void run() override;

private:
    void executeCreation(const SEMPQCreationParams &params);

    QWizard *wizard;
    SEMPQCreator *creator;
    bool cancelRequested;
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
    void reject() override;

private:
    SEMPQIntroPage *introPage;
    SEMPQSettingsPage *settingsPage;
    SEMPQTargetPage *targetPage;
    PluginPage *pluginPage;
    SEMPQProgressPage *progressPage;
};

#endif // SEMPQWIZARD_H
