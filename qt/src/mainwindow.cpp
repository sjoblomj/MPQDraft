/*
    MainWindow - Implementation
*/

#include "mainwindow.h"
#include "patchwizard.h"
#include "sempqwizard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QFont>
#include <QMessageBox>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    } else if (event->key() == Qt::Key_P) {
        onPatchClicked();
    } else if (event->key() == Qt::Key_M) {
        onSEMPQClicked();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::setupUI()
{
    // Set window properties
    setWindowTitle("MPQDraft");
    setFixedSize(420, 300);

    // Create central widget and main layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // Title label
    titleLabel = new QLabel("MPQDraft", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Add some spacing
    mainLayout->addStretch();

    // Patch button
    patchButton = new QPushButton("Load MPQs and &Patch", this);
    patchButton->setMinimumHeight(50);
    QFont buttonFont = patchButton->font();
    buttonFont.setPointSize(12);
    patchButton->setFont(buttonFont);
    connect(patchButton, &QPushButton::clicked, this, &MainWindow::onPatchClicked);
    mainLayout->addWidget(patchButton);

    // SEMPQ button
    sempqButton = new QPushButton("Create Self-Executing &MPQ", this);
    sempqButton->setMinimumHeight(50);
    sempqButton->setFont(buttonFont);
    connect(sempqButton, &QPushButton::clicked, this, &MainWindow::onSEMPQClicked);
    mainLayout->addWidget(sempqButton);

    // Add spacing at bottom
    mainLayout->addStretch();

    // Version label
    QLabel *versionLabel = new QLabel("Qt Version - Development Build", this);
    versionLabel->setAlignment(Qt::AlignCenter);
    QFont versionFont = versionLabel->font();
    versionFont.setPointSize(8);
    versionLabel->setFont(versionFont);
    versionLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(versionLabel);

    setCentralWidget(centralWidget);
}

void MainWindow::onPatchClicked()
{
    // Create and show the patch wizard
    PatchWizard wizard(this);

    // Position wizard at main window location and hide main window
    wizard.move(this->pos());
    this->hide();

    wizard.exec();

    // Show main window again after wizard closes
    this->show();
}

void MainWindow::onSEMPQClicked()
{
    // Create and show the SEMPQ wizard
    SEMPQWizard wizard(this);

    // Position wizard at main window location and hide main window
    wizard.move(this->pos());
    this->hide();

    wizard.exec();

    // Show main window again after wizard closes
    this->show();
}

