/*
    PluginPage - Implementation
*/

#include "pluginpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QLabel>

PluginPage::PluginPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Select Plugins");
    setSubTitle("Choose plugins to load. Plugins can add custom patching functionality.");
    
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // Plugin list with checkboxes
    pluginListWidget = new QListWidget(this);
    mainLayout->addWidget(pluginListWidget);
    
    // Buttons
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    browseButton = new QPushButton("Browse...", this);
    configureButton = new QPushButton("Configure", this);
    configureButton->setEnabled(false);
    
    connect(browseButton, &QPushButton::clicked, this, &PluginPage::onBrowseClicked);
    connect(configureButton, &QPushButton::clicked, this, &PluginPage::onConfigureClicked);
    connect(pluginListWidget, &QListWidget::itemSelectionChanged, 
            this, &PluginPage::onItemSelectionChanged);
    
    buttonLayout->addWidget(browseButton);
    buttonLayout->addWidget(configureButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    // Load plugins from default directory
    loadPluginsFromDirectory();
}

PluginPage::~PluginPage()
{
    // Clean up loaded plugins
    for (PluginInfo *info : loadedPlugins) {
        if (info->hDLLModule) {
            FreeLibrary(info->hDLLModule);
        }
        delete info;
    }
}

QStringList PluginPage::getSelectedPlugins() const
{
    QStringList selected;
    for (int i = 0; i < pluginListWidget->count(); ++i) {
        QListWidgetItem *item = pluginListWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            selected.append(item->data(Qt::UserRole).toString());
        }
    }
    return selected;
}

void PluginPage::loadPluginsFromDirectory()
{
    // Look for plugins in the "plugins" subdirectory
    QDir pluginDir("plugins");
    if (!pluginDir.exists()) {
        return;
    }
    
    // Find all .qdp files (MPQDraft plugin files)
    QStringList filters;
    filters << "*.qdp" << "*.dll";
    QStringList pluginFiles = pluginDir.entryList(filters, QDir::Files);
    
    for (const QString &fileName : pluginFiles) {
        QString fullPath = pluginDir.absoluteFilePath(fileName);
        addPlugin(fullPath);
    }
}

void PluginPage::addPlugin(const QString &path)
{
    // Don't add duplicates
    if (loadedPlugins.contains(path)) {
        return;
    }
    
    // Try to load the plugin
    PluginInfo *info = new PluginInfo();
    if (!LoadPluginInfo(path.toStdString().c_str(), *info)) {
        delete info;
        return;
    }
    
    // Add to our map
    loadedPlugins[path] = info;
    
    // Add to list widget
    QString displayName = QString::fromStdString(info->strPluginName);
    QListWidgetItem *item = new QListWidgetItem(displayName, pluginListWidget);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
    item->setData(Qt::UserRole, path);  // Store full path
}

void PluginPage::onBrowseClicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        "Select Plugin Files",
        QString(),
        "MPQDraft Plugins (*.qdp *.dll);;All Files (*.*)"
    );
    
    for (const QString &fileName : fileNames) {
        addPlugin(fileName);
    }
}

void PluginPage::onConfigureClicked()
{
    QListWidgetItem *currentItem = pluginListWidget->currentItem();
    if (!currentItem) {
        return;
    }
    
    QString pluginPath = currentItem->data(Qt::UserRole).toString();
    PluginInfo *info = loadedPlugins.value(pluginPath);
    
    if (!info || !info->pPlugin) {
        return;
    }
    
    // Get the native window handle for the plugin to use
    // This is how we bridge Qt and the Windows plugin API
    HWND hwnd = (HWND)winId();
    
    // Call the plugin's Configure method
    if (!info->pPlugin->Configure(hwnd)) {
        QMessageBox::warning(this, "Configuration Failed",
                           "Failed to configure the plugin.");
    }
}

void PluginPage::onItemSelectionChanged()
{
    // Enable configure button only if a plugin is selected
    configureButton->setEnabled(pluginListWidget->currentItem() != nullptr);
}

