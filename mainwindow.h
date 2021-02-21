#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ddcdoc.h"
#include "monitorwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    DdcDoc *m_ddcDoc;

    monitorWidget *m_monitoyWidget;
};

#endif // MAINWINDOW_H
