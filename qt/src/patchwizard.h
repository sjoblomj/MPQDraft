/*
    PatchWizard - Wizard for patching a game executable with MPQs and plugins
    
    This wizard has 3 pages:
    1. Select target executable (game to patch)
    2. Select MPQ files to load
    3. Select and configure plugins
*/

#ifndef PATCHWIZARD_H
#define PATCHWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

// Forward declarations
class PluginPage;

//=============================================================================
// Page 1: Select Target Executable
//=============================================================================
class TargetSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit TargetSelectionPage(QWidget *parent = nullptr);
    
    QString getTargetPath() const;
    QString getParameters() const;
    bool useExtendedRedir() const;

private slots:
    void onBrowseClicked();

private:
    QLineEdit *targetPathEdit;
    QLineEdit *parametersEdit;
    QCheckBox *extendedRedirCheck;
    QPushButton *browseButton;
};

//=============================================================================
// Page 2: Select MPQ Files
//=============================================================================
class MPQSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit MPQSelectionPage(QWidget *parent = nullptr);
    
    QStringList getSelectedMPQs() const;

private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();

private:
    QListWidget *mpqListWidget;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *moveUpButton;
    QPushButton *moveDownButton;
};

//=============================================================================
// Main Patch Wizard
//=============================================================================
class PatchWizard : public QWizard
{
    Q_OBJECT

public:
    explicit PatchWizard(QWidget *parent = nullptr);
    
    void accept() override;

private:
    TargetSelectionPage *targetPage;
    MPQSelectionPage *mpqPage;
    PluginPage *pluginPage;
    
    void performPatch();
};

#endif // PATCHWIZARD_H

