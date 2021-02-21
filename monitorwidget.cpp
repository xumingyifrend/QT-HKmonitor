#include "monitorwidget.h"
#pragma execution_character_set("utf-8")

monitorWidget::monitorWidget(DdcDoc *ddcDoc, QWidget *parent):m_ddcDoc(ddcDoc),QWidget(parent)
{
    m_tabWidget = new QTabWidget;
    m_monitorPreviewWidget = new monitorPreviewWidget(m_ddcDoc);
    m_playbackWidget = new playbackWidget(m_ddcDoc);
    m_systemConfigureWidget = new systemConfigureWidget(m_ddcDoc);
    m_tabWidget->addTab(m_monitorPreviewWidget , "监控预览");
    m_tabWidget->addTab(m_playbackWidget , "回放");
    m_tabWidget->addTab(m_systemConfigureWidget , "添加摄像头");
    layout = new QHBoxLayout;
    layout->addWidget(m_tabWidget);

    connect(m_systemConfigureWidget , SIGNAL(addCamera(QString , QString)) , m_monitorPreviewWidget , SLOT(onAddCamera(QString , QString)));
    connect(m_systemConfigureWidget , SIGNAL(addCamera(QString , QString)) , m_playbackWidget,        SLOT(onAddCamera(QString , QString)));
    connect(m_systemConfigureWidget , SIGNAL(cameraStatusChange(QString)) , m_monitorPreviewWidget , SLOT(onCameraStatusChange(QString)));
    connect(m_systemConfigureWidget , SIGNAL(cameraStatusChange(QString)) , m_playbackWidget , SLOT(onCameraStatusChange(QString)));
    connect(m_systemConfigureWidget , SIGNAL(deleteCamera(QString)) , m_monitorPreviewWidget , SLOT(onDeleteCamera(QString)));
    connect(m_systemConfigureWidget , SIGNAL(deleteCamera(QString)) , m_playbackWidget,         SLOT(onDeleteCamera(QString)));

    //m_systemConfigureWidget->connectAllDevice();  //连接已有的摄像头
    setLayout(layout);
}
