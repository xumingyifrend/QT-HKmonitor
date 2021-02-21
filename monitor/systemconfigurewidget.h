#ifndef SYSTEMCONFIGUREWIDGET_H
#define SYSTEMCONFIGUREWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QRegExp>
#include <QHeaderView>
#include "ddcdoc.h"
#include "commoninterface.h"
//#include "rspsdoc.h"

//#include <winsock2.h>
#include <Windows.h>
#pragma comment(lib, "user32.lib")
#include "../HKInclude/plaympeg4.h"
#include "../HKInclude/HCNetSDK.h"

#include <QtConcurrent>

//初始化SDK
bool InitSDK();
static QMap<long , bool> m_cameraStatus;

class cameraTablewidget:public QTableWidget
{
    Q_OBJECT
public:
    cameraTablewidget(QWidget *parent = 0);

    void contextMenuEvent(QContextMenuEvent *event);
    QMenu *pop_menu;
    QAction *action_deleteItem;

    void createActions();
private slots:
    void onDeleteItem();

signals:
    void deleteItem();
};

class cameraView:public QObject
{
    Q_OBJECT
public:
    cameraView();
    ~cameraView();

    QString m_ip = "";

    void msgCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen);
    long loginCamera(QString ip);

private:
    long userID;
    long IRealPlayHandle;
};

class systemConfigureWidget: public QWidget
{
    Q_OBJECT
public:
    explicit systemConfigureWidget(DdcDoc *ddcDoc,QWidget *parent = 0);
    ~systemConfigureWidget();

    bool uninitSDK();
    void cameraReconnect(QString ip);
    void connectAllDevice();
    void loginAllCamera();
private:
    DdcDoc *m_ddcDoc;

    QLabel *m_ipLabel;
    QLineEdit *m_ipLineEdit;
    QPushButton *m_addBtn;
    QHBoxLayout *m_firstHBoxLayout;
    cameraTablewidget *m_tableWidget;
    QVBoxLayout *m_layout;

    QTimer *reinitializeSDKTimer;
    bool initSDKFlag;
    QTimer *listenCameraStatusTimer;

    QVector<cameraView *> m_cameraView;
    QMap<QPushButton*,QString> m_pbToIpMap;

    QFuture<void> m_cameraReconnectFuture;
    QFutureWatcher<void> m_cameraReconnectWatcher;

    QVector<RSPS_SYSTEM_PARA>* rspsAllDeviceSystemPara; //用于读取存好的设备IP

private slots:
    void onAddBtnClicked();
    void onAddBtnClicked(QString ip , QString id);
    void onDeleteItem();
    void onConnectClicked();
    void onConnectClicked(QString ip);
    void onCameraReconnect();

    void onReinitializeSDKTimerOut();
    void onListenCameraStatusTimeOut();

signals:
    void addCamera(QString ip , QString name);
    void cameraStatusChange(QString ip);
    void deleteCamera(QString ip);
};

#endif // SYSTEMCONFIGUREWIDGET_H
