#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>
#include <QTimer>
//#include <winsock2.h>
#include <Windows.h>
#pragma comment(lib, "user32.lib")
#include "../HKInclude/plaympeg4.h"
#include "../HKInclude/HCNetSDK.h"
#include <QGridLayout>
#include "previewitem.h"
#include "ddcdoc.h"

class previewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit previewWidget(DdcDoc *ddcDoc, QWidget *parent = 0);
    ~previewWidget();

    previewItem *m_monitorLabel[16];
    QGridLayout *m_gridLayout;

    void setCameraView(int number = 2);
    bool stopRealPlay(long userId);
    void stopVoiceIntercom(long userId);
    void initCameraView(long userId);
	void initPreviewItem(long userId);

private:
    DdcDoc *m_ddcDoc;

private slots:
    bool onStartPreview(long userId);
    void onVoiceIntercom(long userId , bool voiceFlag);
    void onStopPreview(long userId);
};

#endif // PREVIEWWIDGET_H
