#include "timebar.h"
#include <QPainter>
#include <QDebug>
#include <QMessageBox>
#pragma execution_character_set("utf-8")
timeBar::timeBar(QWidget *parent):QWidget(parent)
{
    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
    setAttribute(Qt::WA_Hover,true);//开启悬停事件
    installEventFilter(this);       //安装事件过滤器
  //  setMouseTracking(true);     //开启鼠标移动事件（默认鼠标按下时才有效）
    hoverFlag = false;
    mousePoint.setX(0);
    mousePoint.setY(0);

    prePoint.setX(0);
    prePoint.setY(0);
    curPoint.setX(0);
    curPoint.setY(0);
    timeLevel = 3;      //默认为半小时
    timeSpace = 30*60;
    pointToSecond = 30;
    startTime = QDateTime::fromString(QDateTime::currentDateTime().toString("yyyy-MM-dd") , "yyyy-MM-dd");  //默认回放时间为当天00:00:00

    m_playbackRecord.clear();
    videoRecordFlag = false;
    mouseDrapBarFlag = false;

    QSize iconSize(10, 10);
    enlargeToolBtn = new QToolButton;
    enlargeToolBtn->setIcon(QPixmap(":/monitor/enlarge.png"));
    enlargeToolBtn->setIconSize(iconSize);
    connect(enlargeToolBtn , SIGNAL(clicked(bool)) , this , SLOT(onEnlargeToolBtnClicked()));

    narrowToolBtn = new QToolButton;
    narrowToolBtn->setIcon(QPixmap(":/monitor/narrow.png"));
    narrowToolBtn->setIconSize(iconSize);
    connect(narrowToolBtn , SIGNAL(clicked(bool)) , this , SLOT(onNarrowToolBtnClicked()));

    m_hlayout = new QHBoxLayout;
    m_hlayout->addStretch();
    m_hlayout->addWidget(enlargeToolBtn);
    m_hlayout->addWidget(narrowToolBtn);
    m_hlayout->setSpacing(2);
    m_hlayout->setContentsMargins(0 , 0 , 0 , 0);

    m_vlayout = new QVBoxLayout;
    m_vlayout->addLayout(m_hlayout);
    m_vlayout->addStretch();
    m_vlayout->setSpacing(0);
    m_vlayout->setContentsMargins(0 , 0 , 0 , 0);
    setLayout(m_vlayout);

    setFixedHeight(60);
}

timeBar::~timeBar()
{

}

//事件过滤器
bool timeBar::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::HoverEnter)
    {
        hoverFlag = true;
        //获取当前的时间节点
        showCurrentTime((QHoverEvent *)event);
        return true;
    }
    else if(event->type() == QEvent::HoverMove)
    {
        hoverFlag = true;
        showCurrentTime((QHoverEvent *)event);
        return true;
    }
    else if(event->type() == QEvent::HoverLeave)
    {
        hoverFlag = false;
        showCurrentTime((QHoverEvent *)event);
        return true;
    }
    return QWidget::eventFilter(obj,event);
}

void timeBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    //画背景色
    painter.setPen(QPen(Qt::black));
    painter.setBrush(QBrush(Qt::black));
    painter.drawRect(QRect(0 , 0 , width() , 40));


    painter.setPen(QPen(Qt::gray));
    painter.setBrush(QBrush(Qt::gray));
    painter.drawRect(QRect(0 , 40 , width() , 44));

    painter.setBrush(Qt::NoBrush);
    //画头部
    //根据有无视频绘制
    if(!videoRecordFlag)
    {
        painter.setPen(QPen(Qt::black));
        painter.setBrush(QBrush(Qt::black));
        painter.drawRect(QRect(0 , 44 , width() , 56));
    }
    else
    {
        CAMERA_PLAYBACK_ITEM_INFO *item;
        for(int i = 0 ; i <m_playbackRecord.size() ; i++)
        {
            int startSpace = 0;
            int endSpace = 0;
            int left = 0;
            int right = 0;

            item = m_playbackRecord.data() + i;
            if(item->flag)
            {
                if(startTime.toTime_t() < item->start.toTime_t())
                {
                    startSpace = (item->start.toTime_t() - startTime.toTime_t())/pointToSecond;
                    endSpace = (item->end.toTime_t() - item->start.toTime_t())/pointToSecond;

                    left = (width()/2)+ startSpace;
                    right = left + endSpace;

                    if((left < 0) && (right < 0))
                    {

                    }
                    else if((left < 0) && (right > 0))
                    {
                        painter.setPen(QPen(Qt::blue));
                        painter.setBrush(QBrush(Qt::blue));
                        painter.drawRect(QRect(0 , 44 , right , 56));
                    }
                    else if((left > 0) && (right < 0))
                    {

                    }
                    else if((left > 0) && (right > 0))
                    {
                        painter.setPen(QPen(Qt::blue));
                        painter.setBrush(QBrush(Qt::blue));
                        painter.drawRect(QRect(left , 44 , right , 56));
                    }
                }
                else
                {
                    startSpace = (startTime.toTime_t() - item->start.toTime_t())/pointToSecond;
                    endSpace = (item->end.toTime_t() - item->start.toTime_t())/pointToSecond;
                    left = (width()/2) - startSpace;
                    right = left + endSpace;

                    if((left < 0) && (right < 0))
                    {

                    }
                    else if((left < 0) && (right > 0))
                    {
                        painter.setPen(QPen(Qt::blue));
                        painter.setBrush(QBrush(Qt::blue));
                        painter.drawRect(QRect(0 , 44 , right , 56));
                    }
                    else if((left > 0) && (right < 0))
                    {

                    }
                    else if((left > 0) && (right > 0))
                    {
                        painter.setPen(QPen(Qt::blue));
                        painter.setBrush(QBrush(Qt::blue));
                        painter.drawRect(QRect(left , 44 , right , 56));
                    }
                }
            }
            else
            {
                if(startTime.toTime_t() < item->start.toTime_t())
                {
                    startSpace = (item->start.toTime_t() - startTime.toTime_t())/pointToSecond;
                    endSpace = (item->end.toTime_t() - item->start.toTime_t())/pointToSecond;

                    left = (width()/2)+ startSpace;
                    right = left + endSpace;

                    if((left < 0) && (right < 0))
                    {

                    }
                    else if((left < 0) && (right > 0))
                    {
                        painter.setPen(QPen(Qt::black));
                        painter.setBrush(QBrush(Qt::black));
                        painter.drawRect(QRect(0 , 44 , right , 56));
                    }
                    else if((left > 0) && (right < 0))
                    {

                    }
                    else if((left > 0) && (right > 0))
                    {
                        painter.setPen(QPen(Qt::black));
                        painter.setBrush(QBrush(Qt::black));
                        painter.drawRect(QRect(left , 44 , right , 56));
                    }
                }
                else
                {
                    startSpace = (startTime.toTime_t() - item->start.toTime_t())/pointToSecond;
                    endSpace = (item->end.toTime_t() - item->start.toTime_t())/pointToSecond;
                    left = (width()/2) - startSpace;
                    right = left + endSpace;

                    if((left < 0) && (right < 0))
                    {

                    }
                    else if((left < 0) && (right > 0))
                    {
                        painter.setPen(QPen(Qt::black));
                        painter.setBrush(QBrush(Qt::black));
                        painter.drawRect(QRect(0 , 44 , right , 56));
                    }
                    else if((left > 0) && (right < 0))
                    {

                    }
                    else if((left > 0) && (right > 0))
                    {
                        painter.setPen(QPen(Qt::black));
                        painter.setBrush(QBrush(Qt::black));
                        painter.drawRect(QRect(left , 44 , right , 56));
                    }
                }
            }

            if(i == 0)
            {
                if(left > 0)
                {
                    if(left > width())
                    {
                        painter.setPen(QPen(Qt::black));
                        painter.setBrush(QBrush(Qt::black));
                        painter.drawRect(QRect(0 , 44 , width() , 56));
                    }
                    else
                    {
                        painter.setPen(QPen(Qt::black));
                        painter.setBrush(QBrush(Qt::black));
                        painter.drawRect(QRect(0 , 44 , left , 56));
                    }
                }
            }

            if(i == (m_playbackRecord.size() - 1))
            {
                if(right < width())
                {
                    if(right < 0)
                    {
                        painter.setPen(QPen(Qt::black));
                        painter.setBrush(QBrush(Qt::black));
                        painter.drawRect(QRect(0 , 44 , width() , 56));
                    }
                    else
                    {
                        painter.setPen(QPen(Qt::black));
                        painter.setBrush(QBrush(Qt::black));
                        painter.drawRect(QRect(right , 44 , width() , 56));
                    }
                }
            }
        }
    }

    painter.setPen(QPen(Qt::gray));
    painter.setBrush(QBrush(Qt::gray));
    painter.drawRect(QRect(0 , 56 , width() , 60));

    painter.setPen(QPen(Qt::red));
    painter.setBrush(QBrush(Qt::red));
    //确定左右第一个位置偏离的位置
    QDateTime firstTime_left;
    QDateTime firstTime_right;
    int offset_left = 0;
    int offset_right = 0;
    int secondSpace = startTime.toTime_t() % timeSpace;
    offset_left = (secondSpace -timeSpace)/pointToSecond;
    firstTime_left = QDateTime::fromTime_t(startTime.toTime_t() - secondSpace + timeSpace);
    offset_right = (timeSpace - secondSpace)/pointToSecond;
    firstTime_right = QDateTime::fromTime_t(startTime.toTime_t() - secondSpace + timeSpace);

    int timeNumber = ((width()/2)/TIMEBAR_SPACE) + 3;
    //以startTime为中心点向两边画图
    for(int i = 0 ; i < timeNumber ; i++)
    {
        int leftDistance = offset_left + TIMEBAR_SPACE*i;
        int rightDistance = offset_right + TIMEBAR_SPACE*i;
        //画左边线
        painter.drawLine(QPointF((width()/2)- leftDistance , 0) , QPointF((width()/2)- leftDistance , 10));
        painter.drawLine(QPointF((width()/2)- leftDistance , TIMEBAR_HEIGHT) , QPointF((width()/2)- leftDistance , TIMEBAR_HEIGHT - 10));
        painter.drawText(QRectF(QPointF(((width()/2)- leftDistance) - 20 , 28) , QPointF(((width()/2)- leftDistance) +20 , 40)) ,
                         QString("%1").arg(QDateTime::fromTime_t(firstTime_left.toTime_t() - (timeSpace*i)).toString("hh:mm")));

        //画右边线
        painter.drawLine(QPointF((width()/2) + rightDistance , 0) , QPointF((width()/2) + rightDistance , 10));
        painter.drawLine(QPointF((width()/2) + rightDistance , TIMEBAR_HEIGHT) , QPointF((width()/2) + rightDistance , TIMEBAR_HEIGHT - 10));
        painter.drawText(QRectF(QPointF(((width()/2) + rightDistance) - 20 , 28) , QPointF(((width()/2) + rightDistance)+20 , 40)) ,
                         QString("%1").arg(QDateTime::fromTime_t(firstTime_right.toTime_t() + (timeSpace*i)).toString("hh:mm")));
    }

    //以startTime为中心点画线
    painter.setPen(QPen(Qt::yellow));
    painter.setBrush(QBrush(Qt::yellow));
    painter.drawLine(QPointF(width()/2 , 0) , QPointF(width()/2 , 60));

    painter.setPen(QPen(Qt::white));
    painter.setBrush(QBrush(Qt::white));
    painter.drawText(QRectF(QPointF(width()/2 - 80 , 0) , QPointF(width()/2 + 80 , 40)) , startTime.toString("yyyy-MM-dd hh:mm:ss"));

    //鼠标所在位置的时间点
    if(hoverFlag)
    {
        painter.setPen(QPen(Qt::white));
        painter.setBrush(QBrush(Qt::white));
        int space = (mousePoint.x() - (width()/2))*pointToSecond; //距离中间点的秒数
        painter.drawText(QRectF(QPointF(mousePoint.x() - 80 , 0) , QPointF(mousePoint.x() + 80 , 40)) ,
                         QDateTime::fromTime_t(startTime.toTime_t() + space).toString("yyyy-MM-dd hh:mm:ss"));
    }
}

QSize timeBar::minimumSizeHint() const
{
    QSize size = QSize(width(),TIMEBAR_HEIGHT);
    return size;
}

void timeBar::mousePressEvent(QMouseEvent *event)
{
    QPoint currentPoint = event->pos();

    prePoint.setX(currentPoint.x());
    prePoint.setY(currentPoint.y());
    curPoint.setX(currentPoint.x());
    curPoint.setY(currentPoint.y());
}

void timeBar::mouseReleaseEvent(QMouseEvent *event)
{
    if(mouseDrapBarFlag)
    {
        //在鼠标有拖动进度条的时候才发送信号
        emit mousePressPlayback(startTime);
    }
    mouseDrapBarFlag = false;
    prePoint.setX(0);
    prePoint.setY(0);
    curPoint.setX(0);
    curPoint.setY(0);
}

void timeBar::mouseMoveEvent(QMouseEvent * event)
{
    mouseDrapBarFlag = true;
    mouseToPoint(event);
    update();
}

void timeBar::mouseToPoint(QMouseEvent *event)
{
    QPoint currentPoint = event->pos();
    if((prePoint.x() == 0) && (prePoint.y() == 0) && (curPoint.x() == 0) && (curPoint.y() == 0))
    {
        prePoint.setX(currentPoint.x());
        prePoint.setY(currentPoint.y());
        curPoint.setX(currentPoint.x());
        curPoint.setY(currentPoint.y());
    }
    else
    {
        prePoint.setX(curPoint.x());
        prePoint.setY(curPoint.y());
        curPoint.setX(currentPoint.x());
        curPoint.setY(currentPoint.y());
    }

    int distance = curPoint.x() - prePoint.x();
    startTime = QDateTime::fromTime_t(startTime.toTime_t() - (distance*pointToSecond));
}

//增加时间间隔
void timeBar::onEnlargeToolBtnClicked()
{
    timeLevel++;
    if(timeLevel > 5)
    {
        timeLevel = 5;
        return;
    }

    switch (timeLevel) {
    case 0:
        timeSpace = 5*60;
        pointToSecond = 5;
        break;
    case 1:
        timeSpace = 10*60;
        pointToSecond = 10;
        break;
    case 2:
        timeSpace = 20*60;
        pointToSecond = 20;
        break;
    case 3:
        timeSpace = 30*60;
        pointToSecond = 30;
        break;
    case 4:
        timeSpace = 60*60;
        pointToSecond = 60;
        break;
    case 5:
        timeSpace = 120*60;
        pointToSecond = 120;
        break;
    default:
        break;
    }

    update();
}

void timeBar::onNarrowToolBtnClicked()
{
    timeLevel--;
    if(timeLevel < 0)
    {
        timeLevel = 0;
        return;
    }

    switch (timeLevel) {
    case 0:
        timeSpace = 5*60;
        pointToSecond = 5;
        break;
    case 1:
        timeSpace = 10*60;
        pointToSecond = 10;
        break;
    case 2:
        timeSpace = 20*60;
        pointToSecond = 20;
        break;
    case 3:
        timeSpace = 30*60;
        pointToSecond = 30;
        break;
    case 4:
        timeSpace = 60*60;
        pointToSecond = 60;
        break;
    case 5:
        timeSpace = 120*60;
        pointToSecond = 120;
        break;
    default:
        break;
    }

    update();
}

void timeBar::showCurrentTime(QHoverEvent *e)
{
    mousePoint.setX(e->pos().x());
    mousePoint.setY(e->pos().x());
    update();
}

void timeBar::setStartTime(QDateTime dateTime)
{
    startTime = dateTime;
    update();
}

QDateTime timeBar::getStartTime()
{
    return startTime;
}

QDateTime timeBar::getEndTime()
{
    CAMERA_PLAYBACK_ITEM_INFO *item = m_playbackRecord.data() + (m_playbackRecord.size() - 1);
    if(item == NULL)
    {
        QMessageBox::about(NULL , "提示" , "没有可播放的视频");
        return startTime;
    }
    QDateTime dateTime = item->end;
    return dateTime;
}

void timeBar::setPlaybackRecord(QVector<CAMERA_PLAYBACK_ITEM_INFO> playbackRecord)
{
    m_playbackRecord.clear();

    if(playbackRecord.size() == 0)
    {
        videoRecordFlag = false;
        update();
        QMessageBox::about(NULL , "提示" , "没有视频文件");
        return;
    }
    else
    {
        videoRecordFlag = true;
        CAMERA_PLAYBACK_ITEM_INFO *playbackItem;
        playbackItem = playbackRecord.data() + 0;
        firstTime = playbackItem->start;
        playbackItem = playbackRecord.data() + (playbackRecord.size() - 1);
        lastTime = playbackItem->end;
    }

    CAMERA_PLAYBACK_ITEM_INFO *item;
    for(int i = 0 ; i < playbackRecord.size() ; i++)
    {
        item = playbackRecord.data() + i;
        CAMERA_PLAYBACK_ITEM_INFO playbackItem;
        playbackItem.start = item->start;
        playbackItem.end = item->end;
        playbackItem.flag = item->flag;
        m_playbackRecord.append(playbackItem);
    }
    update();
}
