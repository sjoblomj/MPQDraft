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

private:
    void setupUI();
    QIcon createButtonIcon(const QString &text, const QSize &size);
    QIcon createSvgButtonIcon(const QString &svgPath, int size, int bottomPadding);

    // UI components
    QPushButton *patchButton;
    QPushButton *sempqButton;
    QPushButton *languageButton;
};

#endif // MAINWINDOW_H
