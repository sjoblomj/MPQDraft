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
    : QWizardPage(parent),
      pluginManager(new PluginManager())
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
}

PluginPage::~PluginPage()
{
    // PluginManager handles cleanup automatically
    delete pluginManager;
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

std::vector<MPQDRAFTPLUGINMODULE> PluginPage::getSelectedPluginModules() const
{
    std::vector<MPQDRAFTPLUGINMODULE> modules;

    for (int i = 0; i < pluginListWidget->count(); ++i) {
        QListWidgetItem *item = pluginListWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            QString path = item->data(Qt::UserRole).toString();
            std::string pathStr = path.toStdString();

            // Get plugin info from the manager
            const PluginInfo* info = pluginManager->getPluginInfo(pathStr);

            if (info) {
                // Create module structure with real plugin metadata
                MPQDRAFTPLUGINMODULE module;
                module.dwComponentID = info->dwPluginID;  // Real plugin ID!
                module.dwModuleID = 0;  // Main plugin module has ID 0
                module.bExecute = 1;    // Mark as executable

                // Copy the file path
                strncpy(module.szModuleFileName, pathStr.c_str(), MPQDRAFT_MAX_PATH - 1);
                module.szModuleFileName[MPQDRAFT_MAX_PATH - 1] = '\0';

                modules.push_back(module);
            }
        }
    }

    return modules;
}

bool PluginPage::isComplete() const
{
    // Get list of selected plugin paths
    std::vector<std::string> selectedPaths;
    for (int i = 0; i < pluginListWidget->count(); ++i) {
        QListWidgetItem *item = pluginListWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            QString pluginPath = item->data(Qt::UserRole).toString();
            selectedPaths.push_back(pluginPath.toStdString());
        }
    }

    // Delegate validation to PluginManager
    PluginManager::ValidationResult result = pluginManager->validateSelection(selectedPaths);
    return result.valid;
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
    // Get list of selected plugin paths
    std::vector<std::string> selectedPaths;
    for (int i = 0; i < pluginListWidget->count(); ++i) {
        QListWidgetItem *item = pluginListWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            QString pluginPath = item->data(Qt::UserRole).toString();
            selectedPaths.push_back(pluginPath.toStdString());
        }
    }

    // Delegate validation to PluginManager
    PluginManager::ValidationResult result = pluginManager->validateSelection(selectedPaths);

    // Update UI based on validation result
    if (!result.valid) {
        QString errorMsg = QString("<font color='#d32f2f'><b>Warning:</b> %1</font>")
                              .arg(QString::fromStdString(result.errorMessage));
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

bool PluginPage::addPlugin(const QString &path, bool showMessages)
{
    // Check if already in list widget (avoid duplicates in UI)
    for (int i = 0; i < pluginListWidget->count(); ++i) {
        QListWidgetItem *item = pluginListWidget->item(i);
        if (item->data(Qt::UserRole).toString() == path) {
            return true;  // Already in list
        }
    }

    // Delegate plugin loading to PluginManager
    std::string stdPath = path.toStdString();
    std::string errorMessage;
    bool success = pluginManager->addPlugin(stdPath, errorMessage);

    if (!success) {
        // Plugin failed to load - add to list in red, but make it non-checkable
        QString displayName = QFileInfo(path).fileName() + " (failed to load)";
        QListWidgetItem *item = new QListWidgetItem(displayName, pluginListWidget);
        // Remove the checkable flag so the checkbox cannot be enabled
        item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
        item->setData(Qt::UserRole, path);
        item->setForeground(Qt::red);
        item->setIcon(QIcon(":/icons/plugin.svg"));

        if (showMessages) {
            QMessageBox::warning(this, "Failed to Load Plugin",
                               QString("Failed to load plugin from:\n%1\n\n%2")
                               .arg(path)
                               .arg(QString::fromStdString(errorMessage)));
        }
        return false;
    }

    // Plugin loaded successfully - get info and add to list
    const PluginInfo *info = pluginManager->getPluginInfo(stdPath);
    if (!info) {
        return false;  // Shouldn't happen
    }

    QString displayName = QString::fromStdString(info->strPluginName);
    if (displayName.isEmpty()) {
        displayName = QFileInfo(path).fileName();
    }

#ifndef _WIN32
    // On non-Windows, show informational message on first manual add
    if (showMessages) {
        QMessageBox::information(this, "Plugin Loading Not Available",
                                QString("Plugin loading is only available on Windows.\n\n"
                                       "The plugin will be added to the list, but cannot be "
                                       "configured or validated.\n\n"
                                       "File: %1")
                                .arg(QFileInfo(path).fileName()));
    }
    // Mark dummy entries as not loaded
    if (!info->pPlugin) {
        displayName += " (not loaded)";
    }
#endif

    QListWidgetItem *item = new QListWidgetItem(displayName, pluginListWidget);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(showMessages ? Qt::Checked : Qt::Unchecked);
    item->setData(Qt::UserRole, path);
    item->setIcon(QIcon(":/icons/plugin.svg"));

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

        // Delegate removal to PluginManager
        pluginManager->removePlugin(pluginPath.toStdString());

        // Remove from list widget
        delete item;
    }

    validatePluginSelection();
    saveSettings();  // Save after removing plugins
}

void PluginPage::onConfigureClicked()
{
    QListWidgetItem *currentItem = pluginListWidget->currentItem();
    if (!currentItem) {
        return;
    }

    QString pluginPath = currentItem->data(Qt::UserRole).toString();

#ifdef _WIN32
    // Get the native window handle for the plugin to use
    // This is how we bridge Qt and the Windows plugin API
    void *hwnd = (void*)winId();

    // Delegate to PluginManager
    if (!pluginManager->configurePlugin(pluginPath.toStdString(), hwnd)) {
        QMessageBox::warning(this, "Configuration Failed",
                           "Failed to configure the plugin.");
    }
#else
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
        // Only enable if the plugin loaded successfully (not failed)
        enableConfigure = !pluginManager->isPluginFailed(pluginPath.toStdString());
    }
    configureButton->setEnabled(enableConfigure);
}
