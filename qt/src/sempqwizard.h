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
#include <QLabel>

// Forward declarations
class PluginPage;

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

private:
    QLineEdit *sempqNameEdit;
    QLineEdit *mpqPathEdit;
    QLineEdit *iconPathEdit;
    QPushButton *browseMPQButton;
    QPushButton *browseIconButton;
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
    SEMPQSettingsPage *settingsPage;
    PluginPage *pluginPage;
    
    void createSEMPQ();
};

#endif // SEMPQWIZARD_H

