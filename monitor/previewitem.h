#ifndef PREVIEWITEM_H
#define PREVIEWITEM_H

#include <QWidget>
#include <QLabel>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QHBoxLayout>
#include <QMimeData>
#include <QToolButton>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QEvent>
#include "cameralist.h"
#include "ddcdoc.h"

class previewControlPanel : public QWidget
{
    Q_OBJECT
public:
    explicit previewControlPanel(QWidget *parent = NULL);
    ~previewControlPanel();

    QToolButton *stopToolBtn;
    QToolButton *voiceToolBtn;
    QToolButton *fullScreenToolBtn;
    QHBoxLayout *m_toolHBoxLayout;

    bool voiceBtnFlag;
    bool fullScreenFlag;

private slots:
    void onVoiceToolBtnClicked();
    void onStopToolBtnClicked();
    void onFullScreenToolBtnClicked();

signals:
    void voiceIntercom(bool flag);
    void stopPreview();
    void fullScreen(bool flag);

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
};

class previewItem : public QWidget
{
    Q_OBJECT
public:
    explicit previewItem(DdcDoc *ddcDoc, QWidget *parent = 0);

    QLabel *m_label;
    QVBoxLayout *m_layout;
    previewControlPanel *m_previewControlPanel;

    long userId;
    long IRealPlayHandle;
    long lVoiceHanle;

    void setPreviewStatus(bool flag);
    void setVoiceStatus(long hanle);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

private:
    DdcDoc *m_ddcDoc;

signals:
    void preview(long id);
    void voiceIntercom(long id , bool flag);
    void stopPreview(long id);

private slots:
    void onVoiceIntercom(bool flag);
    void onFullScreen(bool flag);
    void onStopPreview();
};

#endif // PREVIEWITEM_H
