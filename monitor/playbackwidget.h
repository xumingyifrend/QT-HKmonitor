#ifndef PLAYBACKWIDGET_H
#define PLAYBACKWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMetaType>
#include "ddcdoc.h"

#include <windows.h>
#pragma comment(lib, "user32.lib")
#include "../HKInclude/plaympeg4.h"
#include "../HKInclude/HCNetSDK.h"

#include <QtConcurrent>

#define INVALID_CAMERA_USER_ID	(-1)

namespace Ui {
class playbackWidget;
}
class playbackWidget: public QWidget
{
    Q_OBJECT
public:
    explicit playbackWidget(DdcDoc *ddcDoc, QWidget *parent = 0);
    ~playbackWidget();

private:
    Ui::playbackWidget *ui;
	int m_lPlayHandle;
    DdcDoc *m_ddcDoc;
	bool m_bPause;
	QTimer m_timer;

    QFuture<void> m_queryDateFuture;
    QFutureWatcher<void> m_queryDateWatcher;

    QFuture<void> m_enterQueryDateFuture;
    QFutureWatcher<void> m_enterQueryDateWatcher;

	void getCameraPlaybackInfo(QVector<CAMERA_PLAYBACK_ITEM_INFO> &playbackInfo);
	int getCurCameraUserID();
    void queryDate(QDateTime dateTime);
	CAMERA_ITEM_INFO* getCameraItemInfo(int userId);
	bool getPlaybackTime(QDateTime &dateTime);
	
private slots:
	void onPlayPushButtonClicked();	
	void onStopPushButtonClicked();	
	void onFastPushButtonClicked();	
	void onSlowPushButtonClicked();	
    void onAddCamera(QString ip , QString name);
    void onCameraStatusChange(QString ip);
    void onDeleteCamera(QString ip);

    void onQueryDate();
    void onEnterToolBtnClicked();
    void onMousePressPlayback(QDateTime dateTime);
	void onTimeout();

signals:
    void setStartTime(QDateTime dateTime);
    void setPlaybackRecord(QVector<CAMERA_PLAYBACK_ITEM_INFO> playbackInfo);
};

#endif // PLAYBACKWIDGET_H
