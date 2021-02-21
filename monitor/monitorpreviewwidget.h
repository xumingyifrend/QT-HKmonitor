#ifndef MONITORPREVIEWWIDGET_H
#define MONITORPREVIEWWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "previewwidget.h"
#include "cameralist.h"
#include "ddcdoc.h"

namespace Ui {
class monitorPreviewWidget;
}

class monitorPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit monitorPreviewWidget(DdcDoc *ddcDoc, QWidget *parent = 0);
    ~monitorPreviewWidget();

    long m_userId[16];

private:
    Ui::monitorPreviewWidget *ui;
    DdcDoc *m_ddcDoc;
    QTimer *listenCameraStatusTimer;

    cameraList *m_cameraList;
    previewWidget *m_previewWidget;
    QHBoxLayout *layout;
    QVBoxLayout *m_leftVBoxLayout;

private slots:
    void onViewComboBoxChange(QString text);
    void onListenCameraStatusTimeOut();

public slots:
    void onCameraStatusChange(QString ip);
    void onAddCamera(QString ip  , QString name);
    void onDeleteCamera(QString ip);
};

#endif // MONITORPREVIEWWIDGET_H
