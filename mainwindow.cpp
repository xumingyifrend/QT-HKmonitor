#include "mainwindow.h"
#include "ui_mainwindow.h"
#pragma execution_character_set("utf-8")
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_ddcDoc = new DdcDoc();

    m_monitoyWidget = new monitorWidget(m_ddcDoc);
    setCentralWidget(m_monitoyWidget);
    setWindowTitle("QT集成海康摄像头功能");
}

MainWindow::~MainWindow()
{
    delete ui;
}
