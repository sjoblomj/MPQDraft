/*
    MainWindow - Main menu for MPQDraft

    Provides two main options:
    1. Load MPQs and Patch - Opens the patch wizard
    2. Create Self-Executing MPQ - Opens the SEMPQ wizard
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QIcon>
#include <QString>
#include <QSize>
#include <QTranslator>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onPatchClicked();
    void onSEMPQClicked();
    void onLanguageClicked();

private:
    void setupUI();
    QIcon createButtonIcon(const QString &text, const QSize &size);
    QIcon createSvgButtonIcon(const QString &svgPath, int size, int bottomPadding);

    // Language handling
    void initLanguage();
    bool loadLanguage(const QString &code);
    void retranslateUI();

    // UI components
    QPushButton *patchButton;
    QPushButton *sempqButton;
    QPushButton *languageButton;

    // Currently installed MPQDraft/Qt translators, and the language code they were loaded for
    QTranslator appTranslator;
    QTranslator qtTranslator;
    QString currentLanguageCode;
};

#endif // MAINWINDOW_H
