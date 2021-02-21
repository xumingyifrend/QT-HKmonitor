#include "previewwidget.h"
#include <QDebug>
#pragma execution_character_set("utf-8")

LONG lPort = -1; //全局的播放库port号
void CALLBACK g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer,DWORD dwBufSize, DWORD dwUser)
{
    switch (dwDataType)
    {
    case NET_DVR_SYSHEAD: //系统头
        if (lPort >= 0)
        {
            break;  //该通道取流之前已经获取到句柄，后续接口不需要再调用
        }

        if (!PlayM4_GetPort(&lPort))  //获取播放库未使用的通道号
        {
            break;
        }
        //m_iPort = lPort; //第一次回调的是系统头，将获取的播放库port号赋值给全局port，下次回调数据时即使用此port号播放
        if (dwBufSize > 0)
        {
            if (!PlayM4_SetStreamOpenMode(lPort, STREAME_REALTIME))  //设置实时流播放模式
            {
                break;
            }

            if (!PlayM4_OpenStream(lPort, pBuffer, dwBufSize, 1024*1024)) //打开流接口
            {
                break;
            }

     /*       if (!PlayM4_Play(lPort, m_hWnd)) //播放开始
            {
                break;
            }*/
        }
        break;
    case NET_DVR_STREAMDATA:   //码流数据
        if (dwBufSize > 0 && lPort != -1)
        {
            if (!PlayM4_InputData(lPort, pBuffer, dwBufSize))
            {
                break;
            }
        }
        break;
    default: //其他数据
        if (dwBufSize > 0 && lPort != -1)
        {
            if (!PlayM4_InputData(lPort, pBuffer, dwBufSize))
            {
                break;
            }
        }
        break;
    }
}

previewWidget::previewWidget(DdcDoc *ddcDoc, QWidget *parent)
    : m_ddcDoc(ddcDoc),QWidget(parent)
{
    m_gridLayout = new QGridLayout;
    for(int i = 0 ; i < 4 ; i++)
    {
        for(int j = 0 ; j < 4 ; j++)
        {
            m_monitorLabel[i*4 + j] = new previewItem(m_ddcDoc);
            m_gridLayout->addWidget(m_monitorLabel[i*4 + j] , i , j);
            connect(m_monitorLabel[i*4 + j] , SIGNAL(preview(long)) , this , SLOT(onStartPreview(long)));
            connect(m_monitorLabel[i*4 + j] , SIGNAL(voiceIntercom(long , bool)) , this , SLOT(onVoiceIntercom(long,bool)));
            connect(m_monitorLabel[i*4 + j] , SIGNAL(stopPreview(long)) , this , SLOT(onStopPreview(long)));
        }
    }
    m_gridLayout->setContentsMargins(1 , 1 , 1 , 1);
    m_gridLayout->setSpacing(1);
    setLayout(m_gridLayout);
}

previewWidget::~previewWidget()
{

}

bool previewWidget::onStartPreview(long userId)
{
    if(userId == -1)
    {
        return false;
    }

    //预览
    NET_DVR_PREVIEWINFO struPlayInfo;
    struPlayInfo={0};

    struPlayInfo.hPlayWnd = (HWND)((previewItem *)sender())->m_label->winId();	//需要 SDK 解码时句柄设为有效值，仅取流不解码时可设为空
    struPlayInfo.lChannel = 1;//预览通道号
    struPlayInfo.dwStreamType = 0;//码流类型：0-主码流，1-子码流，2-三码流，3-虚拟码流，以此类推
    struPlayInfo.dwLinkMode = 0;//0- TCP 方式，1- UDP 方式，2- 多播方式，3- RTP 方式，4-RTP/RTSP，5-RSTP/HTTP
    struPlayInfo.bBlocked = 1;//0- 非阻塞取流，1- 阻塞取流

    long IRealPlayHandle = NET_DVR_RealPlay_V40(userId,&struPlayInfo,NULL,NULL);

    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    CAMERA_ITEM_INFO *cameraItem;
    for(int i = 0 ; i < cameraItemInfo->size() ; i++)
    {
        cameraItem = cameraItemInfo->data() + i;
        if(cameraItem->userId == userId)
        {
            cameraItem->IRealPlayHandle = IRealPlayHandle;
            ((previewItem *)sender())->IRealPlayHandle = IRealPlayHandle;
            break;
        }
    }

    if(IRealPlayHandle <0)
    {
        QString text = tr("NET_DVR_RealPlay_V40 error, error number = %1").arg(NET_DVR_GetLastError());
        qDebug() << text;
        return false;
    }
    else
    {
        qDebug() << "NET_DVR_RealPlay_V40 success";
    }

    //注册回调函数
    if (!NET_DVR_SetRealDataCallBack(IRealPlayHandle, g_RealDataCallBack_V30 , 0))
    {
        QString text = tr("NET_DVR_SetRealDataCallBack error, error number = %1").arg(NET_DVR_GetLastError());
        qDebug() << text;
    }

    return true;
}

void previewWidget::setCameraView(int number)
{
    for(int i = 0 ; i < 4 ; i++)
    {
        for(int j = 0 ; j < 4 ; j++)
        {
            m_gridLayout->itemAt(i*4 +j)->widget()->show();

            //停止正在进行的预览
        }
    }

    for(int i = number ; i < 4 ; i++)
    {
        for(int j = 0 ; j < 4 ; j++)
        {
            m_gridLayout->itemAt(i*4 + j)->widget()->hide();
        }
    }

    for(int i = number ; i < 4 ; i++)
    {
        for(int j = 0 ; j < 4 ; j++)
        {
            m_gridLayout->itemAt(i + j*4)->widget()->hide();
        }
    }
}

bool previewWidget::stopRealPlay(long userId)
{
    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    CAMERA_ITEM_INFO *cameraItem;
    long IRealPlayHandle = -1;
    for(int i = 0 ; i < cameraItemInfo->size() ; i++)
    {
        cameraItem = cameraItemInfo->data() + i;
        if(cameraItem->userId == userId)
        {
            IRealPlayHandle = cameraItem->IRealPlayHandle;
            break;
        }
    }

    if(IRealPlayHandle >= 0)
    {
        if(NET_DVR_StopRealPlay(IRealPlayHandle))		//停止预览
        {
            cameraItem->IRealPlayHandle = -1;
			return true;
        }
        else
        {
        	return false;
        }
    }
	else
	{
		return false;
	}
}

void previewWidget::initPreviewItem(long userId)
{	
    for(int i = 0 ; i < 16 ; i++)
    {
        if(m_monitorLabel[i]->userId == userId)
        {
            m_monitorLabel[i]->userId = -1;
		    m_monitorLabel[i]->IRealPlayHandle = -1;
		    m_monitorLabel[i]->lVoiceHanle = -1;
			m_monitorLabel[i]->setVoiceStatus(m_monitorLabel[i]->lVoiceHanle);
			break;
        }
    }
}

void CALLBACK fVoiceDataCallBack(LONG lVoiceComHandle, char *pRecvDataBuffer, DWORD dwBufSize, BYTE byAudioFlag, void* pUser)
{
    printf("receive voice data, %d\n", dwBufSize);
}

void previewWidget::onVoiceIntercom(long userId , bool voiceFlag)
{
    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    CAMERA_ITEM_INFO *cameraItem;
    if(voiceFlag)
    {
        //如果已开启
        for(int i = 0 ; i < cameraItemInfo->size() ; i++)
        {
            cameraItem = cameraItemInfo->data() + i;
            if(cameraItem->userId == userId)
            {
                if(cameraItem->lVoiceHanle > 0)
                {
                    ((previewItem *)sender())->lVoiceHanle = cameraItem->lVoiceHanle;
                    ((previewItem *)sender())->setVoiceStatus(cameraItem->lVoiceHanle);
                    return;
                }
                break;
            }
        }

        //开启语音对讲
        LONG lVoiceHanle;
        lVoiceHanle = NET_DVR_StartVoiceCom_V30(userId, 1,0, fVoiceDataCallBack, NULL);
        if (lVoiceHanle < 0)
        {
            //QString text = tr("onVoiceIntercom error, error number = %1").arg(NET_DVR_GetLastError());
            //qDebug() << text << lVoiceHanle;
            ((previewItem *)sender())->lVoiceHanle = -1;
            ((previewItem *)sender())->setVoiceStatus(((previewItem *)sender())->lVoiceHanle);
            QMessageBox::about(NULL , "提示" , "语音对讲开启失败");
            return;
        }

        ((previewItem *)sender())->lVoiceHanle = lVoiceHanle;
        ((previewItem *)sender())->setVoiceStatus(((previewItem *)sender())->lVoiceHanle);
        for(int i = 0 ; i < cameraItemInfo->size() ; i++)
        {
            cameraItem = cameraItemInfo->data() + i;
            if(cameraItem->userId == userId)
            {
                cameraItem->lVoiceHanle = lVoiceHanle;
            }
        }
    }
    else
    {
        LONG lVoiceHanle;
        for(int i = 0 ; i < cameraItemInfo->size() ; i++)
        {
            cameraItem = cameraItemInfo->data() + i;
            if(cameraItem->userId == userId)
            {
                lVoiceHanle = cameraItem->lVoiceHanle;
            }
        }

        if(lVoiceHanle < 0)
        {
            ((previewItem *)sender())->lVoiceHanle = lVoiceHanle;
            ((previewItem *)sender())->setVoiceStatus(((previewItem *)sender())->lVoiceHanle);
            return;
        }

        //关闭语音对讲
        if (!NET_DVR_StopVoiceCom(lVoiceHanle))
        {
            ((previewItem *)sender())->lVoiceHanle = lVoiceHanle;
            ((previewItem *)sender())->setVoiceStatus(((previewItem *)sender())->lVoiceHanle);
           return;
        }

        cameraItem->lVoiceHanle = -1;
        ((previewItem *)sender())->lVoiceHanle = cameraItem->lVoiceHanle;
        ((previewItem *)sender())->setVoiceStatus(((previewItem *)sender())->lVoiceHanle);
    }

    return;
}

void previewWidget::stopVoiceIntercom(long userId)
{
    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    CAMERA_ITEM_INFO *cameraItem;
    LONG lVoiceHanle;
    for(int i = 0 ; i < cameraItemInfo->size() ; i++)
    {
        cameraItem = cameraItemInfo->data() + i;
        if(cameraItem->userId == userId)
        {
            lVoiceHanle = cameraItem->lVoiceHanle;
        }
    }

    if(lVoiceHanle < 0)
    {
        return;
    }

    //关闭语音对讲
    if (!NET_DVR_StopVoiceCom(lVoiceHanle))
    {
       return;
    }

    cameraItem->lVoiceHanle = -1;
}
/*
 * //广播
void previewWidget::broadcast()
{
    LONG lVoiceHanle;

    for(int index = 0; index < m_byAudioChanNum; index++)
    {
        lVoiceHanle = NET_DVR_AddDVR_V30(userID, index+1);
    }

    QString text = tr("broadcast %1").arg(m_byAudioChanNum);

    lVoiceHanle = NET_DVR_ClientAudioStart_V30(NULL, NULL);
    if (lVoiceHanle < 0)
    {
       return;
    }

    Sleep(10000);  //millisecond

    NET_DVR_ClientAudioStart_V30(NULL, NULL);

}*/
void previewWidget::onStopPreview(long userId)
{
    bool flag = stopRealPlay(userId);
    ((previewItem *)sender())->setPreviewStatus(flag);
}

void previewWidget::initCameraView(long userId)
{
    for(int i = 0 ; i < 16 ; i++)
    {
        if(m_monitorLabel[i]->userId == userId)
        {
            m_monitorLabel[i]->userId = -1;
            m_monitorLabel[i]->IRealPlayHandle = -1;
            m_monitorLabel[i]->lVoiceHanle = -1;

            break;
        }
    }
}
