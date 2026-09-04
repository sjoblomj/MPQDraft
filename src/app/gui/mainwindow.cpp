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
#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QActionGroup>

namespace {

// Every language MPQDraft can be shown in: English (the untranslated source
// text) plus whichever mpqdraft_<code>.qm translation files are found. New
// languages therefore just need a .ts file added under translations/ - no
// code changes here.
QStringList availableLanguageCodes()
{
    QStringList codes = {QStringLiteral("en")};

    const QStringList searchDirs = {
        QApplication::applicationDirPath(),
        QApplication::applicationDirPath() + "/translations",
        QStringLiteral(":/translations"),
    };
    for (const QString &dirPath : searchDirs) {
        const QDir dir(dirPath);
        const QStringList files = dir.entryList({QStringLiteral("mpqdraft_*.qm")}, QDir::Files);
        for (const QString &file : files) {
            const QString base = QFileInfo(file).completeBaseName();
            const QString code = base.mid(QStringLiteral("mpqdraft_").length());
            if (!code.isEmpty() && !codes.contains(code)) {
                codes.append(code);
            }
        }
    }

    // Keep English first, then the rest alphabetically for a stable menu order
    QStringList rest = codes.mid(1);
    rest.sort(Qt::CaseInsensitive);
    return QStringList{codes.first()} + rest;
}

// Language names are shown in their own language regardless of the current
// UI language (e.g. "svenska" stays "svenska" even when the UI is in Korean)
QString nativeLanguageName(const QString &code)
{
    // "en" is our own fixed entry for the untranslated source text, not a
    // real translation, so it isn't tied to any territory. QLocale always
    // resolves a bare "en" to a concrete one anyway (en_US), whose CLDR
    // self-name is disambiguated as "American English" rather than plain
    // "English" - so special-case it instead of asking QLocale.
    if (code == QStringLiteral("en")) {
        return QStringLiteral("English");
    }

    const QString name = QLocale(code).nativeLanguageName();
    if (name.isEmpty()) {
        return code;
    }
    return name.left(1).toUpper() + name.mid(1);
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initLanguage();
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

// Picks the startup language: whatever was last chosen (persisted via
// QSettings), falling back to the system locale, falling back to English if
// neither is one of the languages we actually have a translation for.
void MainWindow::initLanguage()
{
    const QStringList available = availableLanguageCodes();

    QSettings settings;
    QString code = settings.value(QStringLiteral("language")).toString();
    if (code.isEmpty() || !available.contains(code)) {
        code = QLocale().name().left(2);
    }
    if (!available.contains(code)) {
        code = QStringLiteral("en");
    }

    loadLanguage(code);
}

// Swaps in the translators for the given language code (installing none for
// "en", since that's the untranslated source text) and remembers the choice
// for next launch. Returns false if a non-English code couldn't be loaded.
bool MainWindow::loadLanguage(const QString &code)
{
    qApp->removeTranslator(&qtTranslator);
    qApp->removeTranslator(&appTranslator);

    bool ok = true;
    if (code != QStringLiteral("en")) {
        // Qt's own translations for standard dialogs (QFileDialog, QMessageBox, ...)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (qtTranslator.load(QLocale(code), "qt", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
#else
        if (qtTranslator.load(QLocale(code), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
#endif
            qApp->installTranslator(&qtTranslator);
        }

        const QString translationFile = QStringLiteral("mpqdraft_%1").arg(code);
        const QStringList searchPaths = {
            QApplication::applicationDirPath(),
            QApplication::applicationDirPath() + "/translations",
            QStringLiteral(":/translations"),
        };

        ok = false;
        for (const QString &path : searchPaths) {
            if (appTranslator.load(translationFile, path)) {
                qApp->installTranslator(&appTranslator);
                ok = true;
                break;
            }
        }
    }

    currentLanguageCode = code;

    QSettings settings;
    settings.setValue(QStringLiteral("language"), code);

    return ok;
}

// Re-applies tr() everywhere this window doesn't get it for free: the button
// captions are baked into pixmaps rather than plain widget text, so they
// need to be redrawn after a language switch.
void MainWindow::retranslateUI()
{
    setWindowTitle(tr("MPQDraft"));

    sempqButton->setIcon(createButtonIcon(tr("Create SEMPQ"), QSize(162, 33)));
    sempqButton->setAccessibleName(tr("Create SEMPQ"));
    sempqButton->setAccessibleDescription(tr("Create a Self-Executing MPQ file"));
    sempqButton->setToolTip(tr("Create a Self-Executing MPQ file"));

    patchButton->setIcon(createButtonIcon(tr("Load MPQ Patch"), QSize(162, 33)));
    patchButton->setAccessibleName(tr("Load MPQ Patch"));
    patchButton->setAccessibleDescription(tr("Launch a game with MPQ patches or plugins"));
    patchButton->setToolTip(tr("Launch a game with MPQ patches or plugins"));

    languageButton->setAccessibleName(tr("Select Language"));
    languageButton->setAccessibleDescription(tr("Choose the application language"));
    languageButton->setToolTip(tr("Select Language"));
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
    setFixedSize(420, 273);

    // Create central widget with background image
    QWidget *centralWidget = new QWidget(this);

    // Set background image
    QPixmap background(":/images/main.png");
    QPalette palette;
    palette.setBrush(QPalette::Window, background);
    centralWidget->setAutoFillBackground(true);
    centralWidget->setPalette(palette);

    // SEMPQ button (left button); its caption is drawn by retranslateUI()
    // below, since the plate itself is baked into the background image
    sempqButton = new QPushButton(centralWidget);
    sempqButton->setGeometry(18, 226, 162, 33);
    sempqButton->setFlat(true);
    sempqButton->setStyleSheet("QPushButton { border: none; background: transparent; }");
    sempqButton->setIconSize(QSize(162, 33));
    connect(sempqButton, &QPushButton::clicked, this, &MainWindow::onSEMPQClicked);

    // Language button (globe icon, sits on the metal patch above the SEMPQ button)
    languageButton = new QPushButton(centralWidget);
    languageButton->setGeometry(18, 122, 30, 35);
    languageButton->setFlat(true);
    languageButton->setStyleSheet("QPushButton { border: none; background: transparent; }");
    languageButton->setIcon(createSvgButtonIcon(":/icons/globe.svg", 30, 5));
    languageButton->setIconSize(QSize(30, 35));
    connect(languageButton, &QPushButton::clicked, this, &MainWindow::onLanguageClicked);

    // Patch button (right button)
    patchButton = new QPushButton(centralWidget);
    patchButton->setGeometry(240, 226, 162, 33);
    patchButton->setFlat(true);
    patchButton->setStyleSheet("QPushButton { border: none; background: transparent; }");
    patchButton->setIconSize(QSize(162, 33));
    connect(patchButton, &QPushButton::clicked, this, &MainWindow::onPatchClicked);

    setCentralWidget(centralWidget);

    retranslateUI();

    // Clear focus so no button is highlighted on startup
    centralWidget->setFocus();
}

// Shows the language picker below the globe button; selecting an entry
// switches the running app's language immediately.
void MainWindow::onLanguageClicked()
{
    QMenu menu(this);
    QActionGroup *group = new QActionGroup(&menu);
    group->setExclusive(true);

    for (const QString &code : availableLanguageCodes()) {
        QAction *action = menu.addAction(nativeLanguageName(code));
        action->setCheckable(true);
        action->setChecked(code == currentLanguageCode);
        group->addAction(action);
        connect(action, &QAction::triggered, this, [this, code]() {
            if (code != currentLanguageCode) {
                loadLanguage(code);
                retranslateUI();
            }
        });
    }

    menu.exec(languageButton->mapToGlobal(QPoint(0, languageButton->height())));
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
