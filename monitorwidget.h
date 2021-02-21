#ifndef MONITORWIDGET_H
#define MONITORWIDGET_H

#include <QTabWidget>
#include <QHBoxLayout>
#include <QWidget>
#include "ddcdoc.h"
#include "monitor/monitorpreviewwidget.h"
#include "monitor/playbackwidget.h"
#include "monitor/systemconfigurewidget.h"

class monitorWidget: public QWidget
{
    Q_OBJECT
public:
    monitorWidget(DdcDoc *ddcDoc, QWidget *parent = 0);

private:
    QTabWidget *m_tabWidget;
    QHBoxLayout *layout;
    DdcDoc *m_ddcDoc;

    monitorPreviewWidget *m_monitorPreviewWidget;
    playbackWidget *m_playbackWidget;
    systemConfigureWidget *m_systemConfigureWidget;
};

#endif // MONITORWIDGET_H
