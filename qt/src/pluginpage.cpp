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
#include <QSettings>

PluginPage::PluginPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Select Plugins");
    setSubTitle("Choose plugins to load. Plugins can add custom patching functionality.");
    setPixmap(QWizard::LogoPixmap, QPixmap(":/icons/plugin.svg").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

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

void PluginPage::initializePage()
{
    // Load settings first
    loadSettings();

    // Set the button text based on which wizard is using this page
    QString windowTitle = wizard()->windowTitle();

    if (windowTitle.contains("SEMPQ")) {
        // SEMPQ Wizard - this is NOT the last page, so change Next button to "Create SEMPQ"
        wizard()->setButtonText(QWizard::NextButton, "Create &SEMPQ");
    } else if (windowTitle.contains("Patch")) {
        // Patch Wizard - this IS the last page, so change Finish button to "Launch"
        wizard()->setButtonText(QWizard::FinishButton, "&Launch");
    }

    QWizardPage::initializePage();
}

void PluginPage::cleanupPage()
{
    // Reset button text to default when leaving the page
    QString windowTitle = wizard()->windowTitle();

    if (windowTitle.contains("SEMPQ")) {
        // Reset Next button to default "Next >"
        wizard()->setButtonText(QWizard::NextButton, tr("&Next >"));
    } else if (windowTitle.contains("Patch")) {
        // Reset Finish button to default "Finish"
        wizard()->setButtonText(QWizard::FinishButton, tr("&Finish"));
    }

    QWizardPage::cleanupPage();
}

void PluginPage::saveSettings()
{
    QSettings settings;
    QString wizardName = wizard()->windowTitle().contains("SEMPQ") ? "SEMPQWizard" : "PatchWizard";
    settings.beginGroup(wizardName + "/Plugin");

    // Save last plugin directory
    settings.setValue("lastDirectory", lastPluginDirectory);

    // Clear old plugin entries first
    settings.remove("plugins");

    // Save plugin list with checked state (manual index management)
    settings.beginGroup("plugins");
    for (int i = 0; i < pluginListWidget->count(); ++i) {
        settings.beginGroup(QString::number(i));
        QListWidgetItem *item = pluginListWidget->item(i);
        QString pluginPath = item->data(Qt::UserRole).toString();
        settings.setValue("path", pluginPath);
        settings.setValue("checked", item->checkState() == Qt::Checked);
        settings.endGroup();
    }
    settings.endGroup();

    settings.endGroup();
}

void PluginPage::loadSettings()
{
    QSettings settings;
    QString wizardName = wizard()->windowTitle().contains("SEMPQ") ? "SEMPQWizard" : "PatchWizard";
    settings.beginGroup(wizardName + "/Plugin");

    // Block signals while loading to avoid triggering saves
    pluginListWidget->blockSignals(true);

    // Restore last plugin directory
    lastPluginDirectory = settings.value("lastDirectory", QDir::currentPath()).toString();

    // Restore plugin list (manual index management)
    settings.beginGroup("plugins");
    int i = 0;
    while (true) {
        settings.beginGroup(QString::number(i));
        if (!settings.contains("path")) {
            settings.endGroup();
            break;
        }

        QString path = settings.value("path").toString();
        bool checked = settings.value("checked", true).toBool();
        settings.endGroup();

        // Only add if file still exists
        if (QFileInfo::exists(path)) {
            bool added = addPlugin(path, false);  // Don't show messages during load
            if (added) {
                // Find the item we just added and set its check state
                for (int j = 0; j < pluginListWidget->count(); ++j) {
                    QListWidgetItem *item = pluginListWidget->item(j);
                    if (item->data(Qt::UserRole).toString() == path) {
                        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
                        break;
                    }
                }
            }
        }

        ++i;
    }
    settings.endGroup();

    // Unblock signals
    pluginListWidget->blockSignals(false);

    settings.endGroup();
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
    saveSettings();  // Save whenever items change (check state, etc.)
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

        // Add Plugin icon
        QIcon pluginIcon(":/icons/plugin.svg");
        item->setIcon(pluginIcon);

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

    // Add Plugin icon
    QIcon pluginIcon(":/icons/plugin.svg");
    item->setIcon(pluginIcon);

    // Select the item if it was added by the user (showMessages=true)
    if (showMessages) {
        pluginListWidget->setCurrentItem(item);
    }

    return true;
}

void PluginPage::onBrowseClicked()
{
    // Use last directory if available
    QString startDir = lastPluginDirectory.isEmpty() ? QDir::currentPath() : lastPluginDirectory;

    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        "Select Plugin Files",
        startDir,
        "MPQDraft Plugins (*.qdp *.dll);;All Files (*.*)"
    );

    // Save the directory for next time
    if (!fileNames.isEmpty()) {
        lastPluginDirectory = QFileInfo(fileNames.first()).absolutePath();
    }

    for (const QString &fileName : fileNames) {
        addPlugin(fileName, true);  // Show messages and auto-check when user adds plugin
    }

    saveSettings();  // Save after adding plugins
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
    saveSettings();  // Save after removing plugins
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
