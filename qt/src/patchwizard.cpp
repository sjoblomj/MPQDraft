/*
    PatchWizard - Implementation
*/

#include "patchwizard.h"
#include "pluginpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QPixmap>
#include <QPainter>

//=============================================================================
// Page 1: Target Selection
//=============================================================================
TargetSelectionPage::TargetSelectionPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Select Target Executable");
    setSubTitle("Choose the game executable to patch and any command-line parameters.");
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Target path
    QLabel *targetLabel = new QLabel("Target Executable:", this);
    layout->addWidget(targetLabel);
    
    QHBoxLayout *targetLayout = new QHBoxLayout();
    targetPathEdit = new QLineEdit(this);
    targetPathEdit->setPlaceholderText("Path to game executable (e.g., StarCraft.exe)");
    browseButton = new QPushButton("Browse...", this);
    connect(browseButton, &QPushButton::clicked, this, &TargetSelectionPage::onBrowseClicked);
    targetLayout->addWidget(targetPathEdit);
    targetLayout->addWidget(browseButton);
    layout->addLayout(targetLayout);
    
    layout->addSpacing(20);
    
    // Parameters
    QLabel *paramsLabel = new QLabel("Command-line Parameters (optional):", this);
    layout->addWidget(paramsLabel);
    parametersEdit = new QLineEdit(this);
    parametersEdit->setPlaceholderText("e.g., -window -opengl");
    layout->addWidget(parametersEdit);
    
    layout->addSpacing(20);
    
    // Extended redirect option
    extendedRedirCheck = new QCheckBox("Use extended file redirection", this);
    extendedRedirCheck->setToolTip("Redirect file open attempts that explicitly specify an archive");
    layout->addWidget(extendedRedirCheck);
    
    layout->addStretch();
    
    // Register field for validation
    registerField("targetPath*", targetPathEdit);
}

QString TargetSelectionPage::getTargetPath() const
{
    return targetPathEdit->text();
}

QString TargetSelectionPage::getParameters() const
{
    return parametersEdit->text();
}

bool TargetSelectionPage::useExtendedRedir() const
{
    return extendedRedirCheck->isChecked();
}

void TargetSelectionPage::onBrowseClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select Target Executable",
        QString(),
        "Executables (*.exe);;All Files (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        targetPathEdit->setText(fileName);
    }
}

//=============================================================================
// Page 2: MPQ Selection
//=============================================================================
MPQSelectionPage::MPQSelectionPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Select MPQ Files");
    setSubTitle("Add MPQ files to load. Files are loaded in order (later files have higher priority).");
    
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // MPQ list
    mpqListWidget = new QListWidget(this);
    mainLayout->addWidget(mpqListWidget);
    
    // Buttons
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    addButton = new QPushButton("Add...", this);
    removeButton = new QPushButton("Remove", this);
    moveUpButton = new QPushButton("Move Up", this);
    moveDownButton = new QPushButton("Move Down", this);
    
    connect(addButton, &QPushButton::clicked, this, &MPQSelectionPage::onAddClicked);
    connect(removeButton, &QPushButton::clicked, this, &MPQSelectionPage::onRemoveClicked);
    connect(moveUpButton, &QPushButton::clicked, this, &MPQSelectionPage::onMoveUpClicked);
    connect(moveDownButton, &QPushButton::clicked, this, &MPQSelectionPage::onMoveDownClicked);
    
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(moveUpButton);
    buttonLayout->addWidget(moveDownButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
}

QStringList MPQSelectionPage::getSelectedMPQs() const
{
    QStringList mpqs;
    for (int i = 0; i < mpqListWidget->count(); ++i) {
        mpqs.append(mpqListWidget->item(i)->text());
    }
    return mpqs;
}

void MPQSelectionPage::onAddClicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        "Select MPQ Files",
        QString(),
        "MPQ Archives (*.mpq);;All Files (*.*)"
    );
    
    for (const QString &fileName : fileNames) {
        mpqListWidget->addItem(fileName);
    }
}

void MPQSelectionPage::onRemoveClicked()
{
    QList<QListWidgetItem*> selected = mpqListWidget->selectedItems();
    for (QListWidgetItem *item : selected) {
        delete item;
    }
}

void MPQSelectionPage::onMoveUpClicked()
{
    int currentRow = mpqListWidget->currentRow();
    if (currentRow > 0) {
        QListWidgetItem *item = mpqListWidget->takeItem(currentRow);
        mpqListWidget->insertItem(currentRow - 1, item);
        mpqListWidget->setCurrentRow(currentRow - 1);
    }
}

void MPQSelectionPage::onMoveDownClicked()
{
    int currentRow = mpqListWidget->currentRow();
    if (currentRow >= 0 && currentRow < mpqListWidget->count() - 1) {
        QListWidgetItem *item = mpqListWidget->takeItem(currentRow);
        mpqListWidget->insertItem(currentRow + 1, item);
        mpqListWidget->setCurrentRow(currentRow + 1);
    }
}

//=============================================================================
// Main Wizard
//=============================================================================
PatchWizard::PatchWizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle("MPQDraft Patch Wizard");
    setWizardStyle(QWizard::ModernStyle);
    setOption(QWizard::HaveHelpButton, false);

    // Set the wizard sidebar image with margin
    QPixmap originalPixmap(":/images/wizard.png");
    int margin = 10;
    QPixmap pixmapWithMargin(originalPixmap.width() + margin * 2,
                             originalPixmap.height() + margin * 2);
    pixmapWithMargin.fill(Qt::transparent);
    QPainter painter(&pixmapWithMargin);
    painter.drawPixmap(margin, margin, originalPixmap);
    painter.end();
    setPixmap(QWizard::WatermarkPixmap, pixmapWithMargin);

    // Create pages
    targetPage = new TargetSelectionPage(this);
    mpqPage = new MPQSelectionPage(this);
    pluginPage = new PluginPage(this);

    // Add pages
    addPage(targetPage);
    addPage(mpqPage);
    addPage(pluginPage);

    // Set minimum size
    setMinimumSize(600, 500);
}

void PatchWizard::accept()
{
    performPatch();
    QWizard::accept();
}

void PatchWizard::performPatch()
{
    // Get data from pages
    QString targetPath = targetPage->getTargetPath();
    QString parameters = targetPage->getParameters();
    bool extendedRedir = targetPage->useExtendedRedir();
    QStringList mpqs = mpqPage->getSelectedMPQs();
    QStringList plugins = pluginPage->getSelectedPlugins();
    
    // TODO: Call the actual patcher DLL
    // For now, just show a message
    QString message = QString("Patch Configuration:\n\n"
                             "Target: %1\n"
                             "Parameters: %2\n"
                             "Extended Redir: %3\n"
                             "MPQs: %4\n"
                             "Plugins: %5")
                        .arg(targetPath)
                        .arg(parameters.isEmpty() ? "(none)" : parameters)
                        .arg(extendedRedir ? "Yes" : "No")
                        .arg(QString::number(mpqs.count()))
                        .arg(QString::number(plugins.count()));
    
    QMessageBox::information(this, "Patch Ready", 
                            message + "\n\nPatching functionality will be implemented next.");
}
