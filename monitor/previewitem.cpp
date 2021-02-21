#include "previewitem.h"
#pragma execution_character_set("utf-8")
#include <QDebug>

previewControlPanel::previewControlPanel(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    voiceBtnFlag = false;
    fullScreenFlag = false;

    QPalette pal;
    pal.setColor(QPalette::Background, QColor(192,192,192));
    setPalette(pal);
    setAutoFillBackground(true);

    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);
    stopToolBtn = new QToolButton;
    stopToolBtn->setIcon(QPixmap(":/monitor/stop_preview.png"));
    stopToolBtn->setIconSize(iconSize);
    connect(stopToolBtn , SIGNAL(clicked(bool)) , this , SLOT(onStopToolBtnClicked()));

    voiceToolBtn = new QToolButton;
   // voiceToolBtn->setAutoRepeat(true);        //这三句可以使得多次触发
   // voiceToolBtn->setAutoRepeatInterval(33);
   // voiceToolBtn->setAutoRepeatDelay(0);
    voiceToolBtn->setIcon(QPixmap(":/monitor/microphone.png"));
    voiceToolBtn->setIconSize(iconSize);
    connect(voiceToolBtn , SIGNAL(clicked(bool)) , this , SLOT(onVoiceToolBtnClicked()));

    fullScreenToolBtn = new QToolButton;
    fullScreenToolBtn->setIcon(QPixmap(":/monitor/fullScreen.png"));
    fullScreenToolBtn->setIconSize(iconSize);
    connect(fullScreenToolBtn , SIGNAL(clicked(bool)) , this , SLOT(onFullScreenToolBtnClicked()));

    m_toolHBoxLayout = new QHBoxLayout;
    m_toolHBoxLayout->addWidget(stopToolBtn);
    m_toolHBoxLayout->addWidget(voiceToolBtn);
    m_toolHBoxLayout->addWidget(fullScreenToolBtn);
    m_toolHBoxLayout->addStretch();

    setLayout(m_toolHBoxLayout);
    setFixedSize(200 , 50);
}

previewControlPanel::~previewControlPanel()
{

}

void previewControlPanel::onVoiceToolBtnClicked()
{
    voiceBtnFlag = !voiceBtnFlag;
    if(voiceBtnFlag)
    {
        voiceToolBtn->setIcon(QPixmap(":/monitor/stopMicrophone.png"));
    }
    else
    {
        voiceToolBtn->setIcon(QPixmap(":/monitor/microphone.png"));
    }
    emit voiceIntercom(voiceBtnFlag);
}

void previewControlPanel::onStopToolBtnClicked()
{
    emit stopPreview();
}

void previewControlPanel::onFullScreenToolBtnClicked()
{
    fullScreenFlag = !fullScreenFlag;
    if(fullScreenFlag)
    {
        fullScreenToolBtn->setIcon(QPixmap(":/monitor/exitFullScreen.png"));
    }
    else
    {
        fullScreenToolBtn->setIcon(QPixmap(":/monitor/fullScreen.png"));
    }
    emit fullScreen(fullScreenFlag);
}

void previewControlPanel::enterEvent(QEvent *event)
{
    show();
    QWidget::enterEvent(event);
}

void previewControlPanel::leaveEvent(QEvent *event)
{
    hide();
    QWidget::leaveEvent(event);
}

previewItem::previewItem(DdcDoc *ddcDoc, QWidget *parent): m_ddcDoc(ddcDoc),QWidget(parent)
{
    setAcceptDrops(true);

    userId = -1;
    IRealPlayHandle = -1;
    lVoiceHanle = -1;

    m_previewControlPanel = new previewControlPanel(this);
    connect(m_previewControlPanel , SIGNAL(voiceIntercom(bool)) , this , SLOT(onVoiceIntercom(bool)));
    connect(m_previewControlPanel , SIGNAL(fullScreen(bool)) , this , SLOT(onFullScreen(bool)));
    connect(m_previewControlPanel , SIGNAL(stopPreview()) , this , SLOT(onStopPreview()));

    m_label = new QLabel();
    m_label->setText(tr("<font style = 'font-size:14px; color:red;'></font>")
                  + tr("<font style = 'font-family:MicrosoftYaHei; font-style:italic; font-size:40px; color:#F13232;'>Camera</font>"));
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setStyleSheet("color:rgb(255 , 255 , 255);background-color:rgb(0 , 0 , 0)");

    m_layout = new QVBoxLayout;
    m_layout->addWidget(m_label);
    m_layout->setContentsMargins(0 , 0 , 0 , 0);

    setLayout(m_layout);
}

void previewItem::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void previewItem::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(cameraList::cameraMimeType()))
    {
        QByteArray pieceData = event->mimeData()->data(cameraList::cameraMimeType());
        QDataStream dataStream(&pieceData, QIODevice::ReadOnly);
        QPixmap pixmap;
        QString id;
        dataStream >> pixmap >> id;

        //判断设备是否已在预览
        QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
        CAMERA_ITEM_INFO *cameraItem;
        for(int i = 0 ; i < cameraItemInfo->size() ; i++)
        {
            cameraItem = cameraItemInfo->data() + i;
            if(cameraItem->userId == id.toLong())
            {
                if(cameraItem->IRealPlayHandle >= 0)
                {
                    QMessageBox::about(NULL , "提示" , "该设备已在预览框中");
                    return;
                }
            }
        }

        if((userId == -1) && (id != "-1"))
        {
            //判断设备状态
            QVector<CAMERA_ITEM_INFO> *cameraItemInfo = m_ddcDoc->getCameraItemInfo();
            CAMERA_ITEM_INFO *cameraItem;
            bool status = false;
            for(int i = 0 ; i < cameraItemInfo->size() ; i++)
            {
                cameraItem = cameraItemInfo->data() + i;
                if(cameraItem->userId == id.toLong())
                {
                    status = cameraItem->status;
                    break;
                }
            }

            if(status)
            {
                userId = id.toLong();
                emit preview(userId);
            }
        }

        //addPiece(pixmap, location);

        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
    {
        event->ignore();
    }
    //获取通道号后播放
    event->acceptProposedAction();
}

void previewItem::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void previewItem::enterEvent(QEvent*event)
{
    if(userId < 0)
    {
        return;
    }

    if(IRealPlayHandle < 0)
    {
        return;
    }

    //显示控制栏
    //获取父窗口大小
    QRect screenRect = geometry();	//获得父类窗口得大小
    int screeny = screenRect.height();	//父窗口得高

    m_previewControlPanel->show();	//要先show出来，才能获得窗口得准确大小，否则获取到得窗口大小不准确
    int h = m_previewControlPanel->geometry().height();
    m_previewControlPanel->move(this->mapToGlobal(QPoint(0, screeny-h)));

    QWidget::enterEvent(event);
}

void previewItem::leaveEvent(QEvent *event)
{
    m_previewControlPanel->hide();
    QWidget::leaveEvent(event);
}

void previewItem::onVoiceIntercom(bool flag)
{
    if(userId < 0)
    {
        return;
    }

    if(IRealPlayHandle < 0)
    {
        return;
    }

    emit voiceIntercom(userId , flag);
}

void previewItem::setVoiceStatus(long hanle)
{
    if(hanle > -1)
    {
        lVoiceHanle = hanle;

        m_previewControlPanel->voiceBtnFlag = true;
        m_previewControlPanel->voiceToolBtn->setIcon(QPixmap(":/monitor/stopMicrophone.png"));
    }
    else
    {
        lVoiceHanle = -1;
        m_previewControlPanel->voiceBtnFlag = false;
        m_previewControlPanel->voiceToolBtn->setIcon(QPixmap(":/monitor/microphone.png"));
    }
}

void previewItem::onFullScreen(bool flag)
{
    if(userId < 0)
    {
        return;
    }

    if(IRealPlayHandle < 0)
    {
        return;
    }

    if(flag)
    {
        setWindowFlags(Qt::Window);
        showFullScreen();
    }
    else
    {
        setWindowFlags (Qt::SubWindow);
        showNormal();
    }
}

void previewItem::onStopPreview()
{
    if(userId < 0)
    {
        return;
    }

    if(IRealPlayHandle < 0)
    {
        return;
    }

    emit stopPreview(userId);
}

void previewItem::setPreviewStatus(bool flag)
{
    if(flag)
    {
        IRealPlayHandle = -1;
        userId = -1;
        lVoiceHanle = -1;

        m_previewControlPanel->voiceBtnFlag = false;
        if(m_previewControlPanel->voiceBtnFlag)
        {
            m_previewControlPanel->voiceToolBtn->setIcon(QPixmap(":/monitor/stopMicrophone.png"));
        }
        else
        {
            m_previewControlPanel->voiceToolBtn->setIcon(QPixmap(":/monitor/microphone.png"));
        }

        m_previewControlPanel->hide();
    }
    else
    {

    }
}
