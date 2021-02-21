#include "playbackwidget.h"
#include "ui_playbackwidget.h"
#pragma execution_character_set("utf-8")
#include <QMessageBox>

using namespace QtConcurrent;

#define PLAY_BACK_WIDGET_DEBUG  0
playbackWidget::playbackWidget(DdcDoc *ddcDoc, QWidget *parent) :
    QWidget(parent),m_ddcDoc(ddcDoc),
    ui(new Ui::playbackWidget)
{
    ui->setupUi(this);
    ui->queryBtn->setIcon(QIcon(":/monitor/query.png"));
    ui->queryBtn->setText("查询");
    ui->queryBtn->setStyleSheet("QPushButton{border:2px groove gray;"
                   "color: rgb(255, 255, 255);border-radius:10px; "
                   "background-color: rgb(255, 0, 0);}"
                   "QPushButton:pressed{color:rgb(0, 0, 255); background-color: rgb(0, 255, 0);}");
    ui->queryBtn->setFixedHeight(40);
    connect(ui->queryBtn , SIGNAL(clicked(bool)) , this , SLOT(onQueryDate()));

    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);
    ui->enterToolBtn->setIcon(QPixmap(":/monitor/enter.png"));
    ui->enterToolBtn->setIconSize(iconSize);
    ui->enterToolBtn->setStyleSheet("QToolButton{background-color: rgb(51, 122, 183);}"
                                    "QToolButton:pressed{color:rgb(0, 0, 255); background-color: rgb(0, 255, 0);}");
    connect(ui->enterToolBtn , SIGNAL(clicked(bool)) , this , SLOT(onEnterToolBtnClicked()));

    ui->dateTimeSellabel->setStyleSheet("background-color: rgb(170, 170, 170);");
    ui->timeEdit->setFixedHeight(size);

    m_lPlayHandle = -1;
    
    qRegisterMetaType<QVector<CAMERA_PLAYBACK_ITEM_INFO>>("QVector<CAMERA_PLAYBACK_ITEM_INFO>");   // 将不识别的参数结构进行注册
    connect(ui->playPushButton, SIGNAL(clicked(bool)), this, SLOT(onPlayPushButtonClicked()));
    connect(ui->stopPushButton, SIGNAL(clicked(bool)), this, SLOT(onStopPushButtonClicked()));
    connect(ui->fastPushButton, SIGNAL(clicked(bool)), this, SLOT(onFastPushButtonClicked()));
    connect(ui->slowPushButton, SIGNAL(clicked(bool)), this, SLOT(onSlowPushButtonClicked()));
    connect(ui->timeBarWidget , SIGNAL(mousePressPlayback(QDateTime)) , this , SLOT(onMousePressPlayback(QDateTime)));
    connect(this , SIGNAL(setStartTime(QDateTime)) , ui->timeBarWidget , SLOT(setStartTime(QDateTime)));
    connect(this , SIGNAL(setPlaybackRecord(QVector<CAMERA_PLAYBACK_ITEM_INFO>)) ,
            ui->timeBarWidget , SLOT(setPlaybackRecord(QVector<CAMERA_PLAYBACK_ITEM_INFO>)));

    ui->timeEdit->setDisplayFormat("HH:mm:ss");
    ui->calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    ui->calendarWidget->setSelectedDate(QDate::currentDate());
    ui->timeEdit->setTime(QTime(0,0,0));

    ui->playPushButton->setIcon(QIcon(":/monitor/play.png"));
    ui->stopPushButton->setIcon(QIcon(":/monitor/stop.png"));
    ui->fastPushButton->setIcon(QIcon(":/monitor/fast.png"));
    ui->slowPushButton->setIcon(QIcon(":/monitor/slow.png"));

    ui->fastPushButton->setEnabled(false);
    ui->slowPushButton->setEnabled(false);

    m_bPause = false;

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    //m_timer.start(1000);
}

playbackWidget::~playbackWidget()
{
    delete ui;
}

void playbackWidget::onAddCamera(QString ip , QString name)
{
    if(name == "")
    {
        ui->cameraListWidget->addCamera(QPixmap(":/monitor/camera_offline.png"),ip , ip);
    }
    else
    {
        ui->cameraListWidget->addCamera(QPixmap(":/monitor/camera_offline.png"),name , ip);
    }
}

void playbackWidget::onCameraStatusChange(QString ip)
{
    QListWidgetItem *item;
    for(int i = 0; i < ui->cameraListWidget->count(); i++)//遍历所算的ITEM
    {
        item = ui->cameraListWidget->item(i);
        QString text = item->data(Qt::UserRole+2).toString();
        if(text == ip)
        {
            break;
        }
    }

    if(item == NULL)
    {
        return;
    }

    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    for(int i = 0 ; i < cameraItemInfo->size() ; i++)
    {
        CAMERA_ITEM_INFO *cameraItem = cameraItemInfo->data()+i;
        if(cameraItem->ip == ip)
        {
           item->setData(Qt::UserRole+1 , QString::number(cameraItem->userId));
           if(cameraItem->status)
           {
                item->setIcon(QPixmap(":/monitor/camera_online.png"));
                item->setData(Qt::UserRole, QVariant(QPixmap(":/monitor/camera_online.png")));
           }
           else
           {
                item->setIcon(QPixmap(":/monitor/camera_offline.png"));
                item->setData(Qt::UserRole, QVariant(QPixmap(":/monitor/camera_offline.png")));
           }
        }
    }
}

void playbackWidget::onDeleteCamera(QString ip)
{
    QString  deviceIP = "";
    QListWidgetItem *item;
    for(int i = 0; i < ui->cameraListWidget->count(); i++)//遍历所算的ITEM
    {
        item = ui->cameraListWidget->item(i);
        deviceIP = item->data(Qt::UserRole+2).toString();
        if(deviceIP == ip)
        {
            ui->cameraListWidget->removeItemWidget(ui->cameraListWidget->takeItem(i));
            break;
        }
    }

    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    CAMERA_ITEM_INFO *cameraItem;
    for(int i = 0 ; i < cameraItemInfo->size() ; i++)
    {
        cameraItem = cameraItemInfo->data() + i;
        if(cameraItem->ip == ip)
        {
            onStopPushButtonClicked();
        }
    }

    //更新cameraItemInfo
    for(int i = 0 ; i < cameraItemInfo->size() ; i++)
    {
        cameraItem = cameraItemInfo->data() + i;
        if(!cameraItem)
        {
            continue;
        }

        if(cameraItem->ip == ip)
        {
            cameraItemInfo->remove(i);
        }
    }
}

void playbackWidget::onPlayPushButtonClicked()
{
    int cameraUserID = getCurCameraUserID();
    if(INVALID_CAMERA_USER_ID == cameraUserID)
    {
        return;
    }

    //获取开始的时间点
    QDateTime startDateTime = ui->timeBarWidget->getStartTime();
    //获取结束的时间点
    QDateTime endDateTime = ui->timeBarWidget->getEndTime();
        
    if(-1 == m_lPlayHandle)
    {
        NET_DVR_VOD_PARA_V50 struVodPara = {0};

        struVodPara.struBeginTime.wYear         = startDateTime.date().year();
        struVodPara.struBeginTime.byMonth       = startDateTime.date().month();
        struVodPara.struBeginTime.byDay         = startDateTime.date().day();
        struVodPara.struBeginTime.byHour        = startDateTime.time().hour();
        struVodPara.struBeginTime.byMinute      = startDateTime.time().minute();
        struVodPara.struBeginTime.bySecond      = startDateTime.time().second();
        struVodPara.struEndTime.wYear           = endDateTime.date().year();
        struVodPara.struEndTime.byMonth         = endDateTime.date().month();
        struVodPara.struEndTime.byDay           = endDateTime.date().day();
        struVodPara.struEndTime.byHour          = endDateTime.time().hour();
        struVodPara.struEndTime.byMinute        = endDateTime.time().minute();
        struVodPara.struEndTime.bySecond        = endDateTime.time().second();

        struVodPara.struIDInfo.dwChannel        = 1;
        struVodPara.hWnd                        = (HWND)ui->label->winId();

        m_lPlayHandle = NET_DVR_PlayBackByTime_V50(cameraUserID, &struVodPara);
        if (m_lPlayHandle == -1)
        {
            long RetErro=NET_DVR_GetLastError();
            QString text = QString("[onPushButtonClicked]: 错误码=%2,错误信息=%3").arg(RetErro).arg(NET_DVR_GetErrorMsg(&RetErro));
            return;
        }
        
        //设置转码类型
        int iTransType = 0;
        if (NET_DVR_PlayBackControl_V40(m_lPlayHandle, NET_DVR_SET_TRANS_TYPE, &iTransType, NULL))
        {
        }

        if (!NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSTART, 0, NULL))
        {
            long RetErro=NET_DVR_GetLastError();
            QString text = QString("[onPushButtonClicked]: 错误码=%2,错误信息=%3").arg(RetErro).arg(NET_DVR_GetErrorMsg(&RetErro));
            
            NET_DVR_StopPlayBack(m_lPlayHandle);
            m_lPlayHandle = -1;
            return;
        }

		if (!NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYNORMAL, 0, NULL))
        {
            
        }		

        if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSTARTAUDIO, 0, NULL))
        {
            NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYAUDIOVOLUME, (0xffff)/2, NULL);
        }
        else
        {
        }

        unsigned __int64 nTotalLen;
        if (NET_DVR_PlayBackControl_V40(m_lPlayHandle, NET_DVR_PLAYGETTOTALLEN, NULL, 0, &nTotalLen, NULL))
        {
        }

		m_timer.start(1000);
		ui->playPushButton->setIcon(QIcon(":/monitor/pause.png"));
        ui->fastPushButton->setEnabled(true);
        ui->slowPushButton->setEnabled(true);
        m_bPause = false;
    }
    else
    {
		if (m_bPause)
		{
			if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYRESTART, 0, NULL))
			{
                ui->playPushButton->setIcon(QIcon(":/monitor/pause.png"));
                m_bPause = false;

				m_timer.start(1000);
                ui->fastPushButton->setEnabled(true);
                ui->slowPushButton->setEnabled(true);
			}
		}
		else
		{
			if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYPAUSE, 0, NULL))
			{
                ui->playPushButton->setIcon(QIcon(":/monitor/play.png"));
				m_bPause=true;

				m_timer.stop();
                ui->fastPushButton->setEnabled(false);
                ui->slowPushButton->setEnabled(false);                
			}
		}        
    }

    return;
}

void playbackWidget::onStopPushButtonClicked()
{
	m_timer.stop();

    int idx = NET_DVR_GetPlayBackPlayerIndex(m_lPlayHandle);
    
    PlayM4_SetDecCallBack(idx, NULL);
    NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSTOPAUDIO, 0, NULL);
    NET_DVR_StopPlayBack(m_lPlayHandle);
    m_lPlayHandle = -1;  

    ui->fastPushButton->setEnabled(false);
    ui->slowPushButton->setEnabled(false); 

    ui->playPushButton->setIcon(QIcon(":/monitor/play.png"));
    m_bPause = false;
}

void playbackWidget::onFastPushButtonClicked()
{
	if (false == NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYFAST, 0, NULL))
    {
		long RetErro=NET_DVR_GetLastError();
        switch (RetErro) {
        case NET_PLAYM4_ORDER_ERROR:
            QMessageBox::warning(NULL , "快放失败" , "已达到最快速播放");
            break;
        default:
            QMessageBox::warning(NULL , "快放失败" , "快放失败");
            break;
        }
	}
    return;
}

void playbackWidget::onSlowPushButtonClicked()
{
	if (false == NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSLOW, 0, NULL))
    {
		long RetErro=NET_DVR_GetLastError();
        switch (RetErro) {
        case NET_PLAYM4_ORDER_ERROR:
            QMessageBox::warning(NULL , "慢放失败" , "已达到最慢速播放");
            break;
        default:
            QMessageBox::warning(NULL , "慢放失败" , "慢放失败");
            break;
        }
	}
    return;
}

void playbackWidget::onQueryDate()
{
	m_timer.stop();

    int cameraUserID = getCurCameraUserID();
    if(cameraUserID == -1)
    {
        QMessageBox::about(NULL , "提示" , "请先选择已连接的摄像机");
        return;
    }

    QDateTime dateTime;
    QString date = ui->calendarWidget->selectedDate().toString("yyyy-MM-dd");
    dateTime = QDateTime::fromString(QString("%1 00:00:00").arg(date) , "yyyy-MM-dd hh:mm:ss");

    //另起线程去连接
    m_queryDateFuture = run(this , &playbackWidget::queryDate , dateTime);
    m_queryDateWatcher.setFuture(m_queryDateFuture);
}

//查询当前设备的回放文件
//如果有则定位到当天00:00:00,或最早/最晚的记录时间
void playbackWidget::queryDate(QDateTime dateTime)
{
    QVector< CAMERA_PLAYBACK_ITEM_INFO > playbackInfo;
    getCameraPlaybackInfo(playbackInfo);
    emit setPlaybackRecord(playbackInfo);

    //判断最早存视频的时间
    if(playbackInfo.size() == 0)
    {
        emit setStartTime(dateTime);
    }
    else
    {
        CAMERA_PLAYBACK_ITEM_INFO *startItem = playbackInfo.data();
        CAMERA_PLAYBACK_ITEM_INFO *endItem = playbackInfo.data() + (playbackInfo.size() -1);
        if((!startItem) || (!endItem))
        {
            emit setStartTime(dateTime);
            return;
        }
		
        if(dateTime.toTime_t() < startItem->start.toTime_t())
        {
            emit setStartTime(startItem->start);
        }
        else if((dateTime.toTime_t() > startItem->start.toTime_t()) && (dateTime.toTime_t() < endItem->end.toTime_t()))
        {
            emit setStartTime(dateTime);
        }
        else if(dateTime.toTime_t() > endItem->end.toTime_t())
        {
            emit setStartTime(endItem->end);
        }
        else
        {
            emit setStartTime(dateTime);
        }
    }
}

//定位到当前时间点
void playbackWidget::onEnterToolBtnClicked()
{
	m_timer.stop();

    //获取当天日期
    QString date = ui->calendarWidget->selectedDate().toString("yyyy-MM-dd");
    //获取当前时间
    QString time = ui->timeEdit->time().toString("hh:mm:ss");
    QDateTime dateTime = QDateTime::fromString(QString("%1 %2").arg(date).arg(time) , "yyyy-MM-dd hh:mm:ss");

    //另起线程去连接
    m_enterQueryDateFuture = run(this , &playbackWidget::queryDate , dateTime);
    m_enterQueryDateWatcher.setFuture(m_enterQueryDateFuture);
//    ui->timeBarWidget->setStartTime(dateTime);
}

void playbackWidget::onMousePressPlayback(QDateTime dateTime)
{
    ui->timeBarWidget->setStartTime(dateTime);
}

int playbackWidget::getCurCameraUserID()
{
    QListWidgetItem *curItem = ui->cameraListWidget->currentItem();
    if(NULL == curItem)
    {
        return INVALID_CAMERA_USER_ID;
    }
    
    QString cameraIp = curItem->data(Qt::UserRole+2).toString();

    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo(); 
    for(int index = 0 ; index < cameraItemInfo->size() ; index++)
    {
       CAMERA_ITEM_INFO *cameraItem = cameraItemInfo->data() + index;
       if(cameraIp == cameraItem->ip)
       {
            return cameraItem->userId;
       }
    }

    return INVALID_CAMERA_USER_ID;   
}

CAMERA_ITEM_INFO* playbackWidget::getCameraItemInfo(int userId)
{
    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo(); 
    for(int index = 0 ; index < cameraItemInfo->size() ; index++)
    {
       CAMERA_ITEM_INFO *cameraItem = cameraItemInfo->data() + index;
       if(userId == cameraItem->userId)
       {
            return cameraItem;
       }
    }

    return NULL;
}

void playbackWidget::getCameraPlaybackInfo(QVector<CAMERA_PLAYBACK_ITEM_INFO> &playbackInfo)
{
    int cameraUserID = getCurCameraUserID();
    if(INVALID_CAMERA_USER_ID == cameraUserID)
    {
        return;
    }
    
    playbackInfo.clear();
    
    NET_DVR_FILECOND_V50 struFileCondV50;
    memset(&struFileCondV50, 0, sizeof(NET_DVR_FILECOND_V50));
    struFileCondV50.struStreamID.dwChannel = 1;

    QDateTime endTime   = QDateTime::currentDateTime();
    QDateTime startTime = endTime.addDays(-31);
    
    struFileCondV50.struStartTime.wYear     = startTime.date().year();
    struFileCondV50.struStartTime.byMonth   = startTime.date().month();
    struFileCondV50.struStartTime.byDay     = startTime.date().day();
    struFileCondV50.struStartTime.byHour    = startTime.time().hour();
    struFileCondV50.struStartTime.byMinute  = startTime.time().minute();
    struFileCondV50.struStartTime.bySecond  = startTime.time().second();
        
    struFileCondV50.struStopTime.wYear      = endTime.date().year();;
    struFileCondV50.struStopTime.byMonth    = endTime.date().month();
    struFileCondV50.struStopTime.byDay      = endTime.date().day();
    struFileCondV50.struStopTime.byHour     = endTime.time().hour();
    struFileCondV50.struStopTime.byMinute   = endTime.time().minute();
    struFileCondV50.struStopTime.bySecond   = endTime.time().second();

    LONG fileHandle = NET_DVR_FindFile_V50(cameraUserID, &struFileCondV50);
    if (fileHandle < 0)
    {
        QMessageBox::warning(NULL , "回放" , "获取文件列表失败");
        return;
    }

	LONG lRet = -1;
    NET_DVR_FINDDATA_V50 struFileInfo = { 0 };

    CAMERA_PLAYBACK_ITEM_INFO playbackItemInfo;
    QDateTime preEndDateTime, curStartDateTime, curEndDateTime;
	while (1)
	{
        lRet = NET_DVR_FindNextFile_V50(fileHandle, &struFileInfo);
		if (lRet == NET_DVR_FILE_SUCCESS)
		{	
#if PLAY_BACK_WIDGET_DEBUG            
            QString text = tr("%1-%2-%3 %4:%5:%6 - %7-%8-%9 %10:%11:%12")
                .arg(struFileInfo.struStartTime.wYear).arg(struFileInfo.struStartTime.byMonth).arg(struFileInfo.struStartTime.byDay)
                .arg(struFileInfo.struStartTime.byHour).arg(struFileInfo.struStartTime.byMinute).arg(struFileInfo.struStartTime.bySecond)
                .arg(struFileInfo.struStopTime.wYear).arg(struFileInfo.struStopTime.byMonth).arg(struFileInfo.struStopTime.byDay)
                .arg(struFileInfo.struStopTime.byHour).arg(struFileInfo.struStopTime.byMinute).arg(struFileInfo.struStopTime.bySecond); 
            QMessageBox::warning(NULL , "回放" , text);
#endif            
            
            curStartDateTime.setDate(QDate(struFileInfo.struStartTime.wYear, struFileInfo.struStartTime.byMonth,struFileInfo.struStartTime.byDay));
            curStartDateTime.setTime(QTime(struFileInfo.struStartTime.byHour, struFileInfo.struStartTime.byMinute, struFileInfo.struStartTime.bySecond));

            curEndDateTime.setDate(QDate(struFileInfo.struStopTime.wYear, struFileInfo.struStopTime.byMonth,struFileInfo.struStopTime.byDay));
            curEndDateTime.setTime(QTime(struFileInfo.struStopTime.byHour, struFileInfo.struStopTime.byMinute, struFileInfo.struStopTime.bySecond));

            if (preEndDateTime.isNull())
            {
                playbackItemInfo.start = curStartDateTime;
            }
            else if(curStartDateTime != preEndDateTime)                
            {
                playbackItemInfo.end  = preEndDateTime;
                playbackItemInfo.flag = 1;                
                playbackInfo.append(playbackItemInfo);

                playbackItemInfo.start = preEndDateTime;
                playbackItemInfo.end   = curStartDateTime;
                playbackItemInfo.flag  = 0;                
                playbackInfo.append(playbackItemInfo);                

                playbackItemInfo.start = curStartDateTime;
            }
            
            preEndDateTime = curEndDateTime;
            
		}
		else
		{
			if (lRet == NET_DVR_ISFINDING)
			{
				Sleep(5);
				continue;
			}
			if(lRet == NET_DVR_FILE_NOFIND)
			{
                //QMessageBox::warning(NULL , "回放" , "未查找到文件");
				break;
			}
			if (lRet == NET_DVR_NOMOREFILE) 
			{
                playbackItemInfo.end  = preEndDateTime;
                playbackItemInfo.flag = 1;
                playbackInfo.append(playbackItemInfo);

				break;
			}
			else
			{
                //QMessageBox::warning(NULL , "回放" , "由于服务器忙,或网络故障,获取文件列表异常终止");
				break;
			}
		}
	}

	NET_DVR_FindClose_V30(fileHandle);

#if PLAY_BACK_WIDGET_DEBUG
    for(int index = 0; index < playbackInfo.size(); index++)
    {
       CAMERA_PLAYBACK_ITEM_INFO item = playbackInfo.at(index);
           QString text = tr("%1-%2-%3 %4:%5:%6 - %7-%8-%9 %10:%11:%12 - %13")
               .arg(item.start.date().year()).arg(item.start.date().month()).arg(item.start.date().day())
               .arg(item.start.time().hour()).arg(item.start.time().minute()).arg(item.start.time().second())
               .arg(item.end.date().year()).arg(item.end.date().month()).arg(item.end.date().day())
               .arg(item.end.time().hour()).arg(item.end.time().minute()).arg(item.end.time().second())
               .arg(item.flag); 
           QMessageBox::warning(NULL , "回放" , text);        
    }        
#endif      
    return;

}


bool playbackWidget::getPlaybackTime(QDateTime &dateTime)
{
    NET_DVR_TIME osdTime;
    if(-1 != m_lPlayHandle)
    {
        NET_DVR_GetPlayBackOsdTime(m_lPlayHandle, &osdTime);
        dateTime.setDate(QDate(osdTime.dwYear, osdTime.dwMonth,osdTime.dwDay));
        dateTime.setTime(QTime(osdTime.dwHour, osdTime.dwMinute, osdTime.dwSecond));
        return true;
    }

    return false;
}

void playbackWidget::onTimeout()
{
    if(-1 == m_lPlayHandle)
    {
        return;
    }

    QDateTime dateTime;
    if(getPlaybackTime(dateTime))
    {
        ui->timeBarWidget->setStartTime(dateTime);
    }
    return;
}
