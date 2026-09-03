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
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
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

// Renders the button caption onto a copy of the (textless) button bitmap,
// mimicking the engraved-metal look of the original hand-drawn artwork:
// a bold condensed uppercase caption with a soft highlight sitting just
// beneath it. Text color flips to white for the pressed/active states,
// matching how the original bitmaps distinguished up vs. down.
QIcon MainWindow::createButtonIcon(const QString &imagePath, const QString &text)
{
    const QPixmap base(imagePath);
    const QString label = text.toUpper();

    QFont font("Arial");
    font.setBold(true);
    font.setStretch(QFont::SemiCondensed);
    font.setPointSizeF(12.0);

    // Shrink the caption until it fits the bitmap, so longer translations
    // don't overflow the button.
    const int maxTextWidth = base.width() - 12;
    QFontMetrics metrics(font);
    while (metrics.horizontalAdvance(label) > maxTextWidth && font.pointSizeF() > 6.0) {
        font.setPointSizeF(font.pointSizeF() - 0.5);
        metrics = QFontMetrics(font);
    }

    auto renderState = [&](const QColor &textColor) {
        QPixmap pixmap = base;
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setFont(font);

        const QRect rect = pixmap.rect();

        // Soft engraved highlight beneath the glyphs
        painter.setPen(QColor(226, 226, 226));
        painter.drawText(rect.translated(0, 2), Qt::AlignCenter, label);

        painter.setPen(textColor);
        painter.drawText(rect, Qt::AlignCenter, label);
        return pixmap;
    };

    QIcon icon;
    icon.addPixmap(renderState(QColor(14, 14, 14)), QIcon::Normal);
    const QPixmap pressed = renderState(Qt::white);
    icon.addPixmap(pressed, QIcon::Active);
    icon.addPixmap(pressed, QIcon::Selected);
    return icon;
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

    // Load button image and draw its caption on top from the translations
    QIcon sempqIcon = createButtonIcon(":/images/SEMPQButton.png", tr("Create SEMPQ"));
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

    QIcon patchIcon = createButtonIcon(":/images/PatchButton.png", tr("Load MPQ Patch"));
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
