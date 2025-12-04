/*
    PluginPage - Plugin selection and configuration page

    This page is shared by both the Patch Wizard and SEMPQ Wizard.
    It allows users to:
    - Browse for plugins
    - Select which plugins to use
    - Configure individual plugins
*/

#ifndef PLUGINPAGE_H
#define PLUGINPAGE_H

#include <QWizardPage>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "../../core/PluginManager.h"
#include "../../sempq/SEMPQCreator.h"  // For MPQDRAFTPLUGINMODULE

class PluginPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit PluginPage(QWidget *parent = nullptr);
    ~PluginPage();

    std::vector<std::string> getSelectedPluginPaths() const;
    std::vector<MPQDRAFTPLUGINMODULE> getSelectedPluginModules() const;
    bool isComplete() const override;
    void initializePage() override;
    void cleanupPage() override;

private slots:
    void onBrowseClicked();
    void onRemoveClicked();
    void onConfigureClicked();
    void onItemSelectionChanged();
    void onItemChanged(QListWidgetItem *item);

private:
    bool addPlugin(const QString &path, bool showMessages = true);
    void validatePluginSelection();
    void saveSettings();
    void loadSettings();

    QString lastPluginDirectory;

    QListWidget *pluginListWidget;
    QPushButton *browseButton;
    QPushButton *removeButton;
    QPushButton *configureButton;
    QLabel *warningText;

    // Plugin business logic manager
    PluginManager *pluginManager;
};

#endif // PLUGINPAGE_H
