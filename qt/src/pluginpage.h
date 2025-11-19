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
#include <QMap>
#include "common/pluginloader.h"

class PluginPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit PluginPage(QWidget *parent = nullptr);
    ~PluginPage();
    
    QStringList getSelectedPlugins() const;

private slots:
    void onBrowseClicked();
    void onConfigureClicked();
    void onItemSelectionChanged();

private:
    void loadPluginsFromDirectory();
    void addPlugin(const QString &path);
    
    QListWidget *pluginListWidget;
    QPushButton *browseButton;
    QPushButton *configureButton;
    
    // Map of plugin paths to their loaded info
    QMap<QString, PluginInfo*> loadedPlugins;
};

#endif // PLUGINPAGE_H

