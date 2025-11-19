/*
    PluginPage - Implementation
*/

#include "pluginpage.h"
#include "common/patcher.h"
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

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Status label at top
    statusLabel = new QLabel(this);
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    mainLayout->addWidget(statusLabel);

    // Horizontal layout for list and buttons
    QHBoxLayout *contentLayout = new QHBoxLayout();

    // Plugin list with checkboxes
    pluginListWidget = new QListWidget(this);
    connect(pluginListWidget, &QListWidget::itemChanged, this, &PluginPage::onItemChanged);
    connect(pluginListWidget, &QListWidget::itemSelectionChanged,
            this, &PluginPage::onItemSelectionChanged);
    contentLayout->addWidget(pluginListWidget);

    // Buttons
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    browseButton = new QPushButton("Browse...", this);
    configureButton = new QPushButton("Configure", this);
    configureButton->setEnabled(false);

    connect(browseButton, &QPushButton::clicked, this, &PluginPage::onBrowseClicked);
    connect(configureButton, &QPushButton::clicked, this, &PluginPage::onConfigureClicked);

    buttonLayout->addWidget(browseButton);
    buttonLayout->addWidget(configureButton);
    buttonLayout->addStretch();

    contentLayout->addLayout(buttonLayout);
    mainLayout->addLayout(contentLayout);

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

void PluginPage::validatePluginSelection()
{
    // Count selected plugins
    int selectedCount = 0;
    int totalModules = 0;

    for (int i = 0; i < pluginListWidget->count(); ++i) {
        QListWidgetItem *item = pluginListWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            selectedCount++;

            // Each plugin counts as 1 module (the plugin DLL itself)
            totalModules++;

            // Get the plugin info to count auxiliary modules
            QString pluginPath = item->data(Qt::UserRole).toString();
            PluginInfo *info = loadedPlugins.value(pluginPath, nullptr);

#ifdef _WIN32
            // TODO
            // On Windows, we can actually call GetModules to get the count
            if (info && info->pPlugin) {
                DWORD numModules = 0;
                if (info->pPlugin->GetModules(nullptr, &numModules)) {
                    totalModules += numModules;
                }
            }
#else
            // On non-Windows, we can't load plugins, so we can't count modules
            // This validation will only work properly on Windows
#endif
        }
    }

    // Check limits
    bool hasError = false;
    QString errorMsg;

    if (selectedCount > MAX_MPQDRAFT_PLUGINS) {
        errorMsg = QString("<font color='#d32f2f'><b>Warning:</b> Too many plugins selected (%1/%2). "
                          "Please deselect some plugins.</font>")
                      .arg(selectedCount).arg(MAX_MPQDRAFT_PLUGINS);
        hasError = true;
    } else if (totalModules > MAX_AUXILIARY_MODULES) {
        errorMsg = QString("<font color='#d32f2f'><b>Warning:</b> Too many plugin modules (%1/%2). "
                          "Plugins and their auxiliary modules exceed the limit. "
                          "Please deselect some plugins.</font>")
                      .arg(totalModules).arg(MAX_AUXILIARY_MODULES);
        hasError = true;
    }

    if (hasError) {
        statusLabel->setText(errorMsg);
        statusLabel->show();
    } else {
        statusLabel->hide();
    }
}

void PluginPage::onItemChanged(QListWidgetItem *item)
{
    Q_UNUSED(item);
    validatePluginSelection();
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
#ifdef _WIN32
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
#else
    // TODO
    QMessageBox::information(this, "Not Available",
                            "Plugin configuration is only available on Windows.\n\n"
                            "This is a development build for GUI testing.");
#endif
}

void PluginPage::onItemSelectionChanged()
{
    // Enable configure button only if a plugin is selected
    configureButton->setEnabled(pluginListWidget->currentItem() != nullptr);
}
