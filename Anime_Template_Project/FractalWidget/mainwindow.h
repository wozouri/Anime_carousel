#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "fractalwidget.h"
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDoubleValidator>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDebug>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onFractalTypeChanged(int index);
    void onApplyJuliaParams();

private:
    FractalWidget *m_fractalWidget;
    QComboBox *m_fractalTypeComboBox;
    QLineEdit *m_juliaCrLineEdit;
    QLineEdit *m_juliaCiLineEdit;
    QPushButton *m_applyJuliaButton;

    void setupUi();
};

#endif // MAINWINDOW_H
