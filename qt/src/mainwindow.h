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
#include <QVBoxLayout>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onPatchClicked();
    void onSEMPQClicked();

private:
    void setupUI();
    
    // UI components
    QPushButton *patchButton;
    QPushButton *sempqButton;
    QLabel *titleLabel;
};

#endif // MAINWINDOW_H

