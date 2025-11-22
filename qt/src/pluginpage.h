/*
    PluginPage - Plugin selection and configuration page
    
    This page is shared by both the Patch Wizard and SEMPQ Wizard.
    It allows users to:
    - Browse for plugin DLLs
    - Select which plugins to use
    - Configure individual plugins
*/

#ifndef PLUGINPAGE_H
#define PLUGINPAGE_H

#include <QWizardPage>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QMap>
#include <QSet>
#include "common/pluginloader.h"

class PluginPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit PluginPage(QWidget *parent = nullptr);
    ~PluginPage();

    QStringList getSelectedPlugins() const;
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
    void loadPluginsFromDirectory();
    bool addPlugin(const QString &path, bool showMessages = true);
    void validatePluginSelection();

    QListWidget *pluginListWidget;
    QPushButton *browseButton;
    QPushButton *removeButton;
    QPushButton *configureButton;
    QLabel *statusLabel;

    // Map of plugin paths to their loaded info
    QMap<QString, PluginInfo*> loadedPlugins;

    // Set of plugin paths that failed to load
    QSet<QString> failedPlugins;
};

#endif // PLUGINPAGE_H
