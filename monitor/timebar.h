#ifndef TIMEBAR_H
#define TIMEBAR_H

#include <QWidget>
#include <QMouseEvent>
#include <QDateTime>
#include <QEvent>
#include <QHoverEvent>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "globaldef.h"

#define TIMEBAR_HEIGHT 60
#define TIMEBAR_SPACE 60
class timeBar : public QWidget
{
    Q_OBJECT
public:
    timeBar(QWidget *parent = 0);
    ~timeBar();

private:
    int timeLevel;  //记录当前时间间隔级别
    int timeSpace;  //记录时间间隔
    int pointToSecond;  //记录一个点位代表多少秒
    bool hoverFlag;
    QPoint mousePoint;
    QVector<CAMERA_PLAYBACK_ITEM_INFO> m_playbackRecord;

    void showCurrentTime(QHoverEvent *e);

public:
    void  paintEvent(QPaintEvent *event);

    QToolButton *enlargeToolBtn;
    QToolButton *narrowToolBtn;
    QHBoxLayout *m_hlayout;
    QVBoxLayout *m_vlayout;

    QPoint prePoint;
    QPoint curPoint;
    QStringList timeList;
    QDateTime startTime;
    QDateTime firstTime;
    QDateTime lastTime;
    bool videoRecordFlag;
    bool mouseDrapBarFlag;

    QDateTime getStartTime();
    QDateTime getEndTime();

protected:
    QSize minimumSizeHint() const;
    bool eventFilter(QObject * obj, QEvent * event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseToPoint(QMouseEvent *event);

private slots:
    void onEnlargeToolBtnClicked();
    void onNarrowToolBtnClicked();

signals:
    void mousePressPlayback(QDateTime dateTime);

public slots:
    void setStartTime(QDateTime dateTime);
    void setPlaybackRecord(QVector<CAMERA_PLAYBACK_ITEM_INFO> playbackRecord);
};

#endif // TIMEBAR_H
