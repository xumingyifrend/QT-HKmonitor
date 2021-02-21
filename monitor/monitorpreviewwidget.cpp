#include "monitorpreviewwidget.h"
#include "ui_monitorpreviewwidget.h"

monitorPreviewWidget::monitorPreviewWidget(DdcDoc *ddcDoc, QWidget *parent) :
     m_ddcDoc(ddcDoc),QWidget(parent),
    ui(new Ui::monitorPreviewWidget)
{
    ui->setupUi(this);

    m_cameraList = new cameraList(m_ddcDoc);
    m_previewWidget = new previewWidget(m_ddcDoc);

    m_leftVBoxLayout = new QVBoxLayout;
    m_leftVBoxLayout->addWidget(m_cameraList);
    m_leftVBoxLayout->addWidget(ui->groupBox);

    layout = new QHBoxLayout;
    layout->addLayout(m_leftVBoxLayout);
    layout->addWidget(m_previewWidget);
    setLayout(layout);
/*
    m_previewWidget->getUserId(m_userId);
    m_cameraList->clear();
    if(m_userId != NULL)
    {
        for(int i = 0 ; i < 16 ; i++)
        {
            if(m_userId[i] != -1)
            {
                m_cameraList->addCamera(QPixmap(":/monitor/device_camera.png"), QPoint(0, 0) , QString::number(m_userId[i]));
                QListWidgetItem *item = m_cameraList->takeItem(0);
                m_cameraList->insertItem(0, item);
            }
        }
    }
*/
    QStringList itemLevelList;
    itemLevelList << "1*1" << "2*2" << "3*3" << "4*4";
    ui->viewComboBox->addItems(itemLevelList);
    connect(ui->viewComboBox , SIGNAL(currentTextChanged(QString)) , this , SLOT(onViewComboBoxChange(QString)));
    ui->viewComboBox->setCurrentText("2*2");

    listenCameraStatusTimer = new QTimer;
    connect(listenCameraStatusTimer , SIGNAL(timeout()) , this , SLOT(onListenCameraStatusTimeOut()));
    listenCameraStatusTimer->start(1000);
}

monitorPreviewWidget::~monitorPreviewWidget()
{
    delete ui;
}

void monitorPreviewWidget::onViewComboBoxChange(QString text)
{
    if(text == "1*1")
    {
        m_previewWidget->setCameraView(1);
    }
    else if(text == "2*2")
    {
        m_previewWidget->setCameraView(2);
    }
    else if(text == "3*3")
    {
        m_previewWidget->setCameraView(3);
    }
    else if(text == "4*4")
    {
        m_previewWidget->setCameraView(4);
    }
}

void monitorPreviewWidget::onAddCamera(QString ip  , QString name)
{
    if(name == "")
    {
        m_cameraList->addCamera(QPixmap(":/monitor/camera_offline.png"),ip , ip);
    }
    else
    {
        m_cameraList->addCamera(QPixmap(":/monitor/camera_offline.png"),name , ip);
    }


    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    if(!cameraItemInfo)
    {
        return;
    }
    CAMERA_ITEM_INFO cameraItem;
    cameraItem.ip = ip;
    cameraItem.IRealPlayHandle = -1;
    cameraItem.userId = -1;
    cameraItem.status = false;
    cameraItem.adapterId = "";
    cameraItem.adapterName = "";
    cameraItem.change = false;
    cameraItem.lVoiceHanle = -1;
    cameraItemInfo->append(cameraItem);
}

void monitorPreviewWidget::onCameraStatusChange(QString ip)
{
    QListWidgetItem *item;
    for(int i = 0; i < m_cameraList->count(); i++)//遍历所有的ITEM
    {
        item = m_cameraList->item(i);
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

void monitorPreviewWidget::onListenCameraStatusTimeOut()
{
    QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
    CAMERA_ITEM_INFO *cameraItem;

    QListWidgetItem *item;
    for(int i = 0; i < m_cameraList->count(); i++)//遍历所算的ITEM
    {
        item = m_cameraList->item(i);
        long userId = item->data(Qt::UserRole+1).toString().toLong();
        for(int j = 0 ; j < cameraItemInfo->size() ; j++)
        {
            cameraItem = cameraItemInfo->data() + j;
            if(!cameraItem)
            {
                continue;
            }

            if(userId == cameraItem->userId)
            {
                //判断状态是否变化
                if(!cameraItem->change)
                {
                    break;
                }

                if(cameraItem->status)
                {
                    item->setIcon(QPixmap(":/monitor/camera_online.png"));
                    item->setData(Qt::UserRole, QVariant(QPixmap(":/monitor/camera_online.png")));
                }
                else
                {
                    item->setIcon(QPixmap(":/monitor/camera_offline.png"));
                    item->setData(Qt::UserRole, QVariant(QPixmap(":/monitor/camera_offline.png")));

                    //离线则停止语音对讲、预览、并更新内存
                    m_previewWidget->stopRealPlay(cameraItem->userId);
                    m_previewWidget->stopVoiceIntercom(cameraItem->userId);
					m_previewWidget->initPreviewItem(cameraItem->userId);
                    cameraItem->change = false;
                }

                break;
            }
        }
    }
}

void monitorPreviewWidget::onDeleteCamera(QString ip)
{
    QString  deviceIP = "";
    QListWidgetItem *item;
    for(int i = 0; i < m_cameraList->count(); i++)//遍历所算的ITEM
    {
        item = m_cameraList->item(i);
        deviceIP = item->data(Qt::UserRole+2).toString();
        if(deviceIP == ip)
        {
            m_cameraList->removeItemWidget(m_cameraList->takeItem(i));
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
            m_previewWidget->stopVoiceIntercom(cameraItem->userId);
            m_previewWidget->stopRealPlay(cameraItem->userId);
            //将正在预览的值重新初始化
            m_previewWidget->initCameraView(cameraItem->userId);

            break;
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
