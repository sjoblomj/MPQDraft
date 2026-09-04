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
#include <QSvgRenderer>

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

// Renders the button caption onto a transparent canvas, mimicking the
// engraved-metal look of the original hand-drawn artwork: a bold condensed
// uppercase caption with a soft highlight sitting just beneath it. Text
// color flips to white for the pressed/active states, matching how the
// original bitmaps distinguished up vs. down. The button plate itself is
// baked into the main background image, so there is nothing to composite
// onto here beyond the caption.
QIcon MainWindow::createButtonIcon(const QString &text, const QSize &size)
{
    const QString label = text.toUpper();

    QFont font("Arial");
    font.setBold(true);
    font.setStretch(QFont::SemiCondensed);
    font.setPointSizeF(12.0);

    // Shrink the caption until it fits the button, so longer translations
    // don't overflow the plate. boundingRect() (rather than horizontalAdvance())
    // is used because it measures the actual ink extents of the glyphs, which
    // for a bold font can overhang the advance width - horizontalAdvance()
    // alone let long captions like the Swedish "LADDA MPQ-PATCH" clip the plate.
    const int maxTextWidth = size.width() - 12;
    QFontMetrics metrics(font);
    while (metrics.boundingRect(label).width() > maxTextWidth && font.pointSizeF() > 5.0) {
        font.setPointSizeF(font.pointSizeF() - 0.5);
        metrics = QFontMetrics(font);
    }

    auto renderState = [&](const QColor &textColor) {
        QPixmap pixmap(size);
        pixmap.fill(Qt::transparent);
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

// Rasterizes an SVG glyph and gives it the same engraved-metal treatment as
// createButtonIcon: a soft light offset copy sitting beneath a dark (or, when
// pressed, white) main copy. Used for icon-only buttons that sit directly on
// top of a metal patch baked into the background image, so there is no
// separate button bitmap to composite onto.
QIcon MainWindow::createSvgButtonIcon(const QString &svgPath, int size, int bottomPadding)
{
    QSvgRenderer renderer(svgPath);
    // The canvas is taller than the glyph itself so the offset highlight
    // below it (see renderState) has room to render without being clipped.
    QPixmap glyph(size, size + bottomPadding);
    glyph.fill(Qt::transparent);
    QPainter glyphPainter(&glyph);
    glyphPainter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&glyphPainter, QRect(0, 0, size, size));
    glyphPainter.end();

    auto tint = [&](const QColor &color) {
        QPixmap tinted(glyph.size());
        tinted.fill(Qt::transparent);
        QPainter painter(&tinted);
        painter.drawPixmap(0, 0, glyph);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(tinted.rect(), color);
        painter.end();
        return tinted;
    };

    auto renderState = [&](const QColor &color) {
        QPixmap pixmap(glyph.size());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);

        // Soft engraved highlight beneath the glyph
        painter.drawPixmap(0, 2, tint(QColor(226, 226, 226)));

        painter.drawPixmap(0, 0, tint(color));
        painter.end();
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

    // Draw the caption from the translations; the plate itself is baked into the background
    QIcon sempqIcon = createButtonIcon(tr("Create SEMPQ"), QSize(162, 33));
    sempqButton->setIcon(sempqIcon);
    sempqButton->setIconSize(QSize(162, 33));

    // Accessibility improvements
    sempqButton->setAccessibleName(tr("Create SEMPQ"));
    sempqButton->setAccessibleDescription(tr("Create a Self-Executing MPQ file"));
    sempqButton->setToolTip(tr("Create a Self-Executing MPQ file"));

    connect(sempqButton, &QPushButton::clicked, this, &MainWindow::onSEMPQClicked);

    // Language button (globe icon, sits on the metal patch above the SEMPQ button)
    languageButton = new QPushButton(centralWidget);
    languageButton->setGeometry(18, 122, 30, 35);
    languageButton->setFlat(true);
    languageButton->setStyleSheet("QPushButton { border: none; background: transparent; }");

    QIcon languageIcon = createSvgButtonIcon(":/icons/globe.svg", 30, 5);
    languageButton->setIcon(languageIcon);
    languageButton->setIconSize(QSize(30, 35));

    languageButton->setAccessibleName(tr("Select Language"));
    languageButton->setAccessibleDescription(tr("Choose the application language"));
    languageButton->setToolTip(tr("Select Language"));

    // Patch button (right button)
    patchButton = new QPushButton(centralWidget);
    patchButton->setGeometry(240, 226, 162, 33);
    patchButton->setFlat(true);
    patchButton->setStyleSheet("QPushButton { border: none; background: transparent; }");

    QIcon patchIcon = createButtonIcon(tr("Load MPQ Patch"), QSize(162, 33));
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
