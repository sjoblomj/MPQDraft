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
#include <QFileInfo>
#include <QIcon>

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
    pluginListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(pluginListWidget, &QListWidget::itemChanged, this, &PluginPage::onItemChanged);
    connect(pluginListWidget, &QListWidget::itemSelectionChanged,
            this, &PluginPage::onItemSelectionChanged);
    contentLayout->addWidget(pluginListWidget);

    // Buttons
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    browseButton = new QPushButton("Add &Plugin...", this);
    removeButton = new QPushButton("&Remove", this);
    configureButton = new QPushButton("&Configure", this);
    removeButton->setEnabled(false);
    configureButton->setEnabled(false);

    connect(browseButton, &QPushButton::clicked, this, &PluginPage::onBrowseClicked);
    connect(removeButton, &QPushButton::clicked, this, &PluginPage::onRemoveClicked);
    connect(configureButton, &QPushButton::clicked, this, &PluginPage::onConfigureClicked);

    buttonLayout->addWidget(browseButton);
    buttonLayout->addWidget(removeButton);
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

bool PluginPage::isComplete() const
{
    // Count selected plugins and modules
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
            if (info && info->pPlugin) {
                DWORD numModules = 0;
                if (info->pPlugin->GetModules(nullptr, &numModules)) {
                    totalModules += numModules;
                }
            }
        }
    }

    // Page is complete if we don't exceed the limits
    return selectedCount <= MAX_MPQDRAFT_PLUGINS && totalModules <= MAX_AUXILIARY_MODULES;
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
    emit completeChanged();
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

    int failedCount = 0;
    for (const QString &fileName : pluginFiles) {
        QString fullPath = pluginDir.absoluteFilePath(fileName);
        if (!addPlugin(fullPath, false)) {  // Don't show messages during auto-load
            failedCount++;
        }
    }

    // Show a single summary message if any plugins failed to load
    if (failedCount > 0) {
        QMessageBox::warning(this, "Plugin Loading Failed",
                           QString("Failed to load %1 plugin(s) from the plugins directory.\n\n"
                                  "The failed plugins are shown in red in the list. "
                                  "They may be corrupted or missing required dependencies.")
                           .arg(failedCount));
    }
}

bool PluginPage::addPlugin(const QString &path, bool showMessages)
{
    // Don't add duplicates - silently filter them out
    if (loadedPlugins.contains(path) || failedPlugins.contains(path)) {
        return true;  // Already handled
    }

    // Try to load the plugin
    PluginInfo *info = new PluginInfo();
    bool loadSuccess = LoadPluginInfo(path.toStdString().c_str(), *info);

    if (!loadSuccess) {
#ifdef _WIN32
        // On Windows, this is a real error - the plugin DLL couldn't be loaded
        delete info;

        // Mark as failed and add to list in red
        failedPlugins.insert(path);

        QString displayName = QFileInfo(path).fileName() + " (failed to load)";
        QListWidgetItem *item = new QListWidgetItem(displayName, pluginListWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, path);  // Store full path
        item->setForeground(Qt::red);  // Make it red

        // Add DLL icon
        QIcon dllIcon(":/icons/DLL.ico");
        item->setIcon(dllIcon);

        if (showMessages) {
            QMessageBox::warning(this, "Failed to Load Plugin",
                               QString("Failed to load plugin from:\n%1\n\n"
                                      "The file may not be a valid MPQDraft plugin, or it may be "
                                      "missing required dependencies.")
                               .arg(path));
        }
        return false;
#else
        // On Linux, plugins can't be loaded (they're Windows DLLs)
        // But we can still add them to the list for testing the UI
        if (showMessages) {
            QMessageBox::information(this, "Plugin Loading Not Available",
                                    QString("Plugin loading is only available on Windows.\n\n"
                                           "This is a development build for GUI testing. "
                                           "The plugin will be added to the list, but cannot be "
                                           "configured or validated.\n\n"
                                           "File: %1")
                                    .arg(QFileInfo(path).fileName()));
        }

        // Populate the existing PluginInfo as a dummy entry for UI testing
        info->strFileName = path.toStdString();
        info->strPluginName = QFileInfo(path).fileName().toStdString();
        info->dwPluginID = 0;
        info->hDLLModule = nullptr;
        info->pPlugin = nullptr;
        // Fall through to add it to the list
#endif
    }

    // Add to our map
    loadedPlugins[path] = info;

    // Add to list widget
    QString displayName = QString::fromStdString(info->strPluginName);
#ifndef _WIN32
    // On Linux, mark dummy entries as not loaded
    if (!info->pPlugin) {
        displayName += " (not loaded)";
    }
#endif
    QListWidgetItem *item = new QListWidgetItem(displayName, pluginListWidget);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    // Check the plugin by default when added by user (showMessages=true)
    // Leave unchecked when auto-loaded from directory (showMessages=false)
    item->setCheckState(showMessages ? Qt::Checked : Qt::Unchecked);
    item->setData(Qt::UserRole, path);  // Store full path

    // Add DLL icon
    QIcon dllIcon(":/icons/DLL.ico");
    item->setIcon(dllIcon);

    // Select the item if it was added by the user (showMessages=true)
    if (showMessages) {
        pluginListWidget->setCurrentItem(item);
    }

    return true;
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
        addPlugin(fileName, true);  // Show messages and auto-check when user adds plugin
    }
}

void PluginPage::onRemoveClicked()
{
    QList<QListWidgetItem*> selectedItems = pluginListWidget->selectedItems();

    for (QListWidgetItem *item : selectedItems) {
        QString pluginPath = item->data(Qt::UserRole).toString();

        // Remove from loaded plugins map and clean up
        if (loadedPlugins.contains(pluginPath)) {
            PluginInfo *info = loadedPlugins[pluginPath];
            if (info->hDLLModule) {
                FreeLibrary(info->hDLLModule);
            }
            delete info;
            loadedPlugins.remove(pluginPath);
        }

        // Remove from failed plugins set if it's there
        failedPlugins.remove(pluginPath);

        // Remove from list widget
        delete item;
    }

    validatePluginSelection();
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
    int selectedCount = pluginListWidget->selectedItems().count();

    // Remove button: enabled if 1 or more items selected
    removeButton->setEnabled(selectedCount >= 1);

    // Configure button: enabled only if exactly 1 item selected AND it's not a failed plugin
    bool enableConfigure = false;
    if (selectedCount == 1) {
        QListWidgetItem *item = pluginListWidget->currentItem();
        QString pluginPath = item->data(Qt::UserRole).toString();
        // Only enable if the plugin loaded successfully
        enableConfigure = !failedPlugins.contains(pluginPath);
    }
    configureButton->setEnabled(enableConfigure);
}
