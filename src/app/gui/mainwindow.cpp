/*
    MainWindow - Main menu for MPQDraft

    Provides two main options:
    1. Load MPQs and Patch - Opens the patch wizard
    2. Create Self-Executing MPQ - Opens the SEMPQ wizard
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
    setWindowTitle(tr("MPQDraft"));
    setFixedSize(420, 273);

    // Create central widget with background image
    QWidget *centralWidget = new QWidget(this);

    // Set background image
    QPixmap background(":/images/main.png");
    QPalette palette;
    palette.setBrush(QPalette::Window, background);
    centralWidget->setAutoFillBackground(true);
    centralWidget->setPalette(palette);

    // SEMPQ button (left button)
    sempqButton = new QPushButton(centralWidget);
    sempqButton->setGeometry(18, 226, 162, 33);
    sempqButton->setFlat(true);
    sempqButton->setStyleSheet("QPushButton { border: none; background: transparent; }");

    // Load button images
    QIcon sempqIcon;
    sempqIcon.addPixmap(QPixmap(":/images/SEMPQButtonUp.png"), QIcon::Normal);
    sempqIcon.addPixmap(QPixmap(":/images/SEMPQButtonDown.png"), QIcon::Active);
    sempqIcon.addPixmap(QPixmap(":/images/SEMPQButtonDown.png"), QIcon::Selected);
    sempqButton->setIcon(sempqIcon);
    sempqButton->setIconSize(QSize(162, 33));

    // Accessibility improvements
    sempqButton->setAccessibleName(tr("Create SEMPQ"));
    sempqButton->setAccessibleDescription(tr("Create a Self-Executing MPQ file"));
    sempqButton->setToolTip(tr("Create a Self-Executing MPQ file"));

    connect(sempqButton, &QPushButton::clicked, this, &MainWindow::onSEMPQClicked);

    // Patch button (right button)
    patchButton = new QPushButton(centralWidget);
    patchButton->setGeometry(240, 226, 162, 33);
    patchButton->setFlat(true);
    patchButton->setStyleSheet("QPushButton { border: none; background: transparent; }");

    QIcon patchIcon;
    patchIcon.addPixmap(QPixmap(":/images/PatchButtonUp.png"), QIcon::Normal);
    patchIcon.addPixmap(QPixmap(":/images/PatchButtonDown.png"), QIcon::Active);
    patchIcon.addPixmap(QPixmap(":/images/PatchButtonDown.png"), QIcon::Selected);
    patchButton->setIcon(patchIcon);
    patchButton->setIconSize(QSize(162, 33));

    // Accessibility improvements
    patchButton->setAccessibleName(tr("Load MPQ Patch"));
    patchButton->setAccessibleDescription(tr("Launch a game with MPQ patches or plugins"));
    patchButton->setToolTip(tr("Launch a game with MPQ patches or plugins"));

    connect(patchButton, &QPushButton::clicked, this, &MainWindow::onPatchClicked);

    setCentralWidget(centralWidget);

    // Clear focus so no button is highlighted on startup
    centralWidget->setFocus();
}

void MainWindow::onPatchClicked()
{
    // Create wizard without parent so it gets its own taskbar entry on Windows
    PatchWizard wizard(nullptr);

    // Position wizard at main window location and hide main window
    wizard.move(this->pos());
    this->hide();

    wizard.exec();

    // Show main window again after wizard closes
    this->show();
}

void MainWindow::onSEMPQClicked()
{
    // Create wizard without parent so it gets its own taskbar entry on Windows
    SEMPQWizard wizard(nullptr);

    // Position wizard at main window location and hide main window
    wizard.move(this->pos());
    this->hide();

    wizard.exec();

    // Show main window again after wizard closes
    this->show();
}
