#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "hexagonwidget.h"
#include <QSlider>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSizeSliderChanged(int value);
    void onSpacingSliderChanged(int value);

private:
    HexagonWidget *m_hexagonWidget;
    QSlider *m_sizeSlider;
    QSlider *m_spacingSlider;
    QLabel *m_sizeLabel;
    QLabel *m_spacingLabel;
};

#endif // MAINWINDOW_H
