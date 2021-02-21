#include "systemconfigurewidget.h"
#pragma execution_character_set("utf-8")
using namespace QtConcurrent;

cameraTablewidget::cameraTablewidget(QWidget *parent):QTableWidget(parent)
{
    createActions();
}

void cameraTablewidget::createActions()
{
    pop_menu = new QMenu;
    action_deleteItem = new QAction(this);
    action_deleteItem->setText("删除");
    connect(action_deleteItem , SIGNAL(triggered(bool)) , this , SLOT(onDeleteItem()));
}

void cameraTablewidget::onDeleteItem()
{
    emit deleteItem();
}

void cameraTablewidget::contextMenuEvent(QContextMenuEvent *event)
{
    pop_menu->clear();
    pop_menu->addAction(action_deleteItem);

    QTableWidgetItem  *curItem =  this->itemAt(event->pos());
    if (NULL == curItem)
    {
        return;
    }

    if(this->currentRow() < 0)
    {
        return;
    }
    pop_menu->exec(QCursor::pos());
    event->accept();
}

bool InitSDK()
{
    bool isInit;

    isInit = NET_DVR_Init();
    if(!isInit)
    {
        QString text = "[InitSDK]: Init camerawidget::InitSDK fail......";
        return isInit;
    }
    else
    {
        QString text = "[InitSDK]: Init camerawidget::InitSDK succeed......";
    }

    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);
    return isInit;
}

void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
    qDebug() << "dwType" << dwType;
    switch(dwType)
    {
        case EXCEPTION_EXCHANGE:
            m_cameraStatus.insert(lUserID , false);  //用户交互异常
        case EXCEPTION_RECONNECT:    //预览时重连
            //离线
            m_cameraStatus.insert(lUserID , false);  //不存在则插入，存在则替换
            break;

        case PREVIEW_RECONNECTSUCCESS:
            m_cameraStatus.insert(lUserID , true);  //预览时重连成功
            break;
        case RELOGIN_SUCCESS:
            m_cameraStatus.insert(lUserID , true);  //用户重登陆成功
        case RESUME_EXCHANGE:
            //在线
        qDebug() << "在线";
            m_cameraStatus.insert(lUserID , true);      //用户交互恢复
            break;
        default:
            break;
    }
}

cameraView::cameraView()
{
}

cameraView::~cameraView()
{

}

long cameraView::loginCamera(QString ip)
{
    m_ip = ip;
    QString userName = "admin";

    //设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, NULL,g_ExceptionCallBack, NULL);

    int port = 8000;
    QString password = "ieslab123";
    NET_DVR_DEVICEINFO_V30 DeviceInfoTmp;
    userID = NET_DVR_Login_V30(ip.toLatin1().data() , port, userName.toLatin1().data(), password.toLatin1().data() ,&DeviceInfoTmp);
    if(userID < 0)
    {
        QString text = tr("NET_DVR_Login_V30 error, %1").arg(NET_DVR_GetLastError());
       // QMessageBox::about(NULL , "提示" , text);
    }
    else
    {
        qDebug() << "NET_DVR_Login_V30 success";
    }

 //   m_byAudioChanNum = DeviceInfoTmp.byAudioChanNum;
    return userID;
}

systemConfigureWidget::systemConfigureWidget(DdcDoc *ddcDoc, QWidget *parent) :
    m_ddcDoc(ddcDoc),QWidget(parent)
{
    m_ipLabel = new QLabel("IP");
    m_ipLineEdit = new QLineEdit;       //用正则表达式对IP做限制
    QRegExp regExpIP("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    m_ipLineEdit->setValidator(new QRegExpValidator(regExpIP , this));
    m_ipLineEdit->setPlaceholderText("请输入摄像头IP");
    m_addBtn = new QPushButton("添加");
    connect(m_addBtn , SIGNAL(clicked(bool)) , this , SLOT(onAddBtnClicked()));
    m_firstHBoxLayout = new QHBoxLayout;
    m_firstHBoxLayout->addWidget(m_ipLabel);
    m_firstHBoxLayout->addWidget(m_ipLineEdit);
    m_firstHBoxLayout->addWidget(m_addBtn);
    m_firstHBoxLayout->addStretch();

    m_tableWidget = new cameraTablewidget;
    CommonInterface::setTableDefaultStyle(m_tableWidget);
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);//设置表格内容自动适应宽度
#else
    m_tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);//设置表格内容自动适应宽度
#endif
    QStringList header;
    header<<"摄像头IP" <<"连接状态";
    CommonInterface::setTableHeader(m_tableWidget, header);
    connect(m_tableWidget , SIGNAL(deleteItem()) , this , SLOT(onDeleteItem()));

    m_layout = new QVBoxLayout;
    m_layout->addLayout(m_firstHBoxLayout);
    m_layout->addWidget(m_tableWidget);
    setLayout(m_layout);

    initSDKFlag = InitSDK();	//SDK只需初始化一次
    reinitializeSDKTimer = new QTimer;
    connect(reinitializeSDKTimer , SIGNAL(timeout()) , this , SLOT(onReinitializeSDKTimerOut()));
    if(!initSDKFlag)		//SDK初始化失败
    {
        reinitializeSDKTimer->start(10000);
    }

    listenCameraStatusTimer = new QTimer;
    connect(listenCameraStatusTimer , SIGNAL(timeout()) , this , SLOT(onListenCameraStatusTimeOut()));
    listenCameraStatusTimer->start(10*1000);

    QObject::connect(&m_cameraReconnectWatcher, SIGNAL(finished()), this, SLOT(onCameraReconnect()));

    //自动添加
}

systemConfigureWidget::~systemConfigureWidget()
{
    uninitSDK();
}

void systemConfigureWidget::onReinitializeSDKTimerOut()
{
    initSDKFlag = InitSDK();	//SDK只需初始化一次
    if(!initSDKFlag)		//初始化失败
    {

    }
    else
    {
        reinitializeSDKTimer->stop();
    }
}

//清除SDK
bool systemConfigureWidget::uninitSDK()
{
    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    for(int i = 0 ; i < cameraItemInfo->size() ; i++)
    {
        CAMERA_ITEM_INFO *item = cameraItemInfo->data()+i;
        if(!item)
        {
            if(item->IRealPlayHandle >= 0)
            {
                NET_DVR_StopVoiceCom(item->lVoiceHanle);      //停止语音对讲
            }

            if(item->IRealPlayHandle >= 0)
            {
                NET_DVR_StopRealPlay(item->IRealPlayHandle);  //停止预览
            }

            if(item->userId >= 0)
            {
                NET_DVR_Logout(item->userId);		//注销设备
            }
        }
    }
    bool isok = NET_DVR_Cleanup();		//释放SDK资源

    if(!isok)
    {
        QString text = "[uninitSDK]: unInit camerawidget::uninitSDK fail......";
    }
    else
    {
        QString text = "[uninitSDK]: unInit camerawidget::uninitSDK succeed......";
    }
    return isok;
}

void systemConfigureWidget::connectAllDevice()
{
    if(rspsAllDeviceSystemPara == NULL)
    {
        return;
    }

    RSPS_SYSTEM_PARA *item;
    for(int i = 0 ; i < rspsAllDeviceSystemPara->size() ; i++)
    {
        item = rspsAllDeviceSystemPara->data() + i;
        if(item == NULL)
        {
            continue;
        }

        if(item->cameraIP == "")
        {
            continue;
        }

        //连接
        onAddBtnClicked(item->cameraIP , item->hostID);
    }

    //另起线程去逐个登录摄像机
    m_cameraReconnectFuture = run(this , &systemConfigureWidget::loginAllCamera);
    m_cameraReconnectWatcher.setFuture(m_cameraReconnectFuture);
}

void systemConfigureWidget::loginAllCamera()
{
    if(rspsAllDeviceSystemPara == NULL)
    {
        return;
    }

    RSPS_SYSTEM_PARA *item;
    for(int i = 0 ; i < rspsAllDeviceSystemPara->size() ; i++)
    {
        item = rspsAllDeviceSystemPara->data() + i;
        if(item == NULL)
        {
            continue;
        }

        if(item->cameraIP == "")
        {
            continue;
        }

        //连接
        onConnectClicked(item->cameraIP);
    }
}

void systemConfigureWidget::cameraReconnect(QString ip)
{
    QString cameraIp = ip;
    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    CAMERA_ITEM_INFO *cameraItem;
    bool flag = false;
    for(int i = 0 ; i < cameraItemInfo->size() ; i++)
    {
        cameraItem = cameraItemInfo->data() + i;
        if(cameraItem != NULL)
        {
            if(cameraItem->ip == cameraIp)
            {
                flag = true;
                break;
            }
        }
    }

    if(!flag)
    {
        return;
    }

    //登录设备
    cameraView *item = new cameraView;
    cameraItem->userId = item->loginCamera(cameraIp);     //更新连接状态的唯一标识
    if(cameraItem->userId == -1)
    {
        cameraItem->status = false;
    }
    else
    {
        cameraItem->status = true;
    }
    m_cameraView.append(item);

    QMap<QPushButton *,QString>::iterator it; //遍历map
    QPushButton *btn;
    for ( it = m_pbToIpMap.begin(); it != m_pbToIpMap.end(); ++it )
    {
        if(it.value() == cameraIp)
        {
            btn = it.key();
            break;
        }
    }

    if(btn == NULL)
    {
        return;
    }

    if(cameraItem->userId == -1)
    {
        if(btn != NULL)
        {
            btn->setText("未连接");
            btn->setEnabled(true);
        }
    }
    else
    {
        if(btn != NULL)
        {
            btn->setText("已连接");
            btn->setEnabled(false);
        }
    }
    emit cameraStatusChange(cameraIp);
}

void systemConfigureWidget::onAddBtnClicked()
{
    //判断当前IP是否已添加过
    int count = m_tableWidget->rowCount();
    for(int i = 0 ; i < count ; i++)
    {
        QString text = m_tableWidget->item(i , 0)->text();
        if(text == m_ipLineEdit->text())
        {
            QMessageBox::about(NULL , "提示" , "该IP在系统中已添加");
            return;
        }
    }
    m_tableWidget->insertRow(count);
    m_tableWidget->setItem(count, 0, new QTableWidgetItem(tr("%1").arg(m_ipLineEdit->text())));

    QPushButton* connectButton = new QPushButton("未连接");
    connectButton->setEnabled(true);
    /*connectButton->setStyleSheet("QPushButton{border:2px groove gray;"
                   "color: rgb(255, 255, 255);border-radius:10px; "
                   "background-color: rgb(51, 122, 183);}"
                   "QPushButton:pressed{color:rgb(0, 0, 255); background-color: rgb(0, 255, 0);}");*/
    //connectButton->setFixedSize(80, 30);
    m_tableWidget->setCellWidget(count,1,connectButton);
    connect(connectButton,SIGNAL(clicked(bool)), this, SLOT(onConnectClicked()));
    if(!m_pbToIpMap.contains(connectButton))
    {
        m_pbToIpMap.insert(connectButton , m_ipLineEdit->text());
    }

    emit addCamera(m_ipLineEdit->text() , "");   //成功添加一个摄像头，但此时尚未连接
}

void systemConfigureWidget::onAddBtnClicked(QString ip , QString id)
{
    //判断当前IP是否已添加过
    int count = m_tableWidget->rowCount();
    for(int i = 0 ; i < count ; i++)
    {
        QString text = m_tableWidget->item(i , 0)->text();
        if(text == m_ipLineEdit->text())
        {
            QMessageBox::about(NULL , "提示" , "该IP在系统中已添加");
            return;
        }
    }
    m_tableWidget->insertRow(count);
    m_tableWidget->setItem(count, 0, new QTableWidgetItem(tr("%1").arg(ip)));

    QString deviceName = "";
    QPushButton* connectButton = new QPushButton("未连接");
    connectButton->setEnabled(true);
    /*connectButton->setStyleSheet("QPushButton{border:2px groove gray;"
                   "color: rgb(255, 255, 255);border-radius:10px; "
                   "background-color: rgb(51, 122, 183);}"
                   "QPushButton:pressed{color:rgb(0, 0, 255); background-color: rgb(0, 255, 0);}");*/
    //connectButton->setFixedSize(80, 30);
    m_tableWidget->setCellWidget(count,1,connectButton);
    connect(connectButton,SIGNAL(clicked(bool)), this, SLOT(onConnectClicked()));
    if(!m_pbToIpMap.contains(connectButton))
    {
        m_pbToIpMap.insert(connectButton , ip);
    }

    emit addCamera(ip , deviceName);   //成功添加一个摄像头，但此时尚未连接
}

void systemConfigureWidget::onDeleteItem()
{
    int row = m_tableWidget->currentRow();

    if(row < 0)
    {
        return;
    }

    QString ip = m_tableWidget->item(row , 0)->text();
    //更新m_pbToIpMap
    m_pbToIpMap.remove(m_pbToIpMap.key(ip));
    //更新m_cameraView
    cameraView *cameraViewItem;
    for(int i = 0 ; i < m_cameraView.size() ; i++)
    {
        cameraViewItem = *(m_cameraView.data() + i);
        if(!cameraViewItem)
        {
            continue;
        }

        if(cameraViewItem->m_ip == ip)
        {
            m_cameraView.remove(i);
            delete cameraViewItem;
            cameraViewItem = NULL;
        }
    }
    //更新m_cameraStatus
    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    CAMERA_ITEM_INFO *cameraItem;
    long userId = -1;
    for(int i = 0 ; i < cameraItemInfo->size() ; i++)
    {
        cameraItem = cameraItemInfo->data() + i;
        if(cameraItem != NULL)
        {
            if(cameraItem->ip == ip)
            {
                userId = cameraItem->userId;
                break;
            }
        }
    }

    if(userId != -1)
    {
        QMap<long , bool>::iterator it;
        for ( it = m_cameraStatus.begin(); it != m_cameraStatus.end(); ++it )
        {
            if(it.key() == userId)
            {
                m_cameraStatus.remove(userId);
            }
        }
    }


    //更新table界面和list界面
    m_tableWidget->removeRow(row);
    emit deleteCamera(ip);
}

void systemConfigureWidget::onConnectClicked()
{
    QMap<QPushButton *,QString>::iterator it = m_pbToIpMap.find(qobject_cast<QPushButton *>(sender()));
    //关闭之前的连接
    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    CAMERA_ITEM_INFO *cameraItem;
    for(int i = 0 ; i < cameraItemInfo->size() ; i++)
    {
        cameraItem = cameraItemInfo->data() + i;
        if(cameraItem != NULL)
        {
            if(cameraItem->ip == it.value())
            {
                if(cameraItem->userId != -1)
                {
                    //登出设备并清理内存
                }

                cameraReconnect(it.value());
                break;
            }
        }
    }
}

void systemConfigureWidget::onConnectClicked(QString ip)
{
    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    CAMERA_ITEM_INFO *cameraItem;
    for(int i = 0 ; i < cameraItemInfo->size() ; i++)
    {
        cameraItem = cameraItemInfo->data() + i;
        if(cameraItem != NULL)
        {
            if(cameraItem->ip == ip)
            {
                if(cameraItem->userId != -1)
                {
                    //登出设备并清理内存
                }

                cameraReconnect(ip);
                break;
            }
        }
    }
}

void systemConfigureWidget::onListenCameraStatusTimeOut()
{
    QMap<long ,bool>::iterator it; //遍历map
    QPushButton *btn;

    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    CAMERA_ITEM_INFO *cameraItem;
    for ( it = m_cameraStatus.begin(); it != m_cameraStatus.end(); ++it )
    {
        for(int i = 0 ; i < cameraItemInfo->size() ; i++)
        {
            cameraItem = cameraItemInfo->data() + i;
            if(cameraItem != NULL)
            {
                if(cameraItem->userId == it.key())
                {
                    //判断状态是否改变
                    if(cameraItem->status == it.value())
                    {
                        break;
                    }

                    cameraItem->status = it.value();
                    cameraItem->change = true;
                    btn = m_pbToIpMap.key(cameraItem->ip);
                    if(!btn)
                    {
                        break;
                    }

					emit cameraStatusChange(cameraItem->ip);
                    if(cameraItem->status)
                    {
                        btn->setEnabled(false);
                        btn->setText("已连接");
                    }
                    else
                    {
                        btn->setEnabled(true);
                        btn->setText("未连接");
                    }
                    break;
                }
            }
        }
    }
}

void systemConfigureWidget::onCameraReconnect()
{

}
